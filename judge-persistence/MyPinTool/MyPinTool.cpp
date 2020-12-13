#define BOOST_NO_CXX11_NULLPTR
#include <fcntl.h>
#include <libpmem.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/icl/interval_map.hpp>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>

#include "pin.H"

enum pmem_status
{
    modified,
    writeback
};

using addr_t = uint64_t;
using namespace boost::icl;
using state_t = boost::icl::interval_map<addr_t, int,
                                         partial_enricher, std::less>;
typedef state_t::interval_type ival;
typedef void TraceFunc(RTN rtn);

/* ================================================================== */
// Global variables
/* ================================================================== */

std::unordered_map<std::string, TraceFunc *> FuncMap;
std::map<addr_t, uint64_t> latest;
std::multimap<addr_t, uint64_t> write_log;
state_t state;
char *addr = nullptr;

const uint64_t mask = 0xfffffffffffffff8;

uint64_t pmem_start = __UINT64_MAX__, pmem_end;

int cnt = 0;

int try_cnts;
void flush_func(RTN rtn);
void drain_func(RTN rtn);
void memmap_func(RTN rtn);
void final_func(RTN rtn);

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<int> KnobEnableRead(KNOB_MODE_WRITEONCE, "pintool",
                         "r", "", "stage counts");

/* ===================================================================== */
// Utilities
/* ===================================================================== */

bool in_pmem(addr_t addr)
{
    return addr >= pmem_start && addr < pmem_end;
}

void setbuckup(uint64_t tid, addr_t size_ptr)
{
    pmem_end = size_ptr;
}

/* ===================================================================== */
// Analysis routines
/* ===================================================================== */

void record_flush(void *return_ip, uint64_t tid, addr_t dst_addr, uint64_t size)
{
    if (!in_pmem(dst_addr))
    {
        return;
    }
    printf("[flush] addr: %lx size: %lu\n", dst_addr, size);
    state += make_pair(ival::right_open(dst_addr & mask,
                                        (dst_addr + size + 0x7) & mask),
                       (int)pmem_status::writeback);
}

void make_test()
{
    printf("============start test===================\n");
    if (write_log.size() > 1)
    {
        for (auto iter = write_log.begin(); iter != write_log.end(); iter++)
        {
            if (rand() % 2 == 0)
            {
                *(uint64_t *)(iter->first) = iter->second;
            }
        }
    }
    exit(0);
}

void record_drain(void *return_ip, uint64_t tid)
{
    printf("[drain]\n");
    PIN_StopApplicationThreads(tid);
    for (auto iter = state.begin(); iter != state.end(); iter++)
    {
        auto rng = iter->first;
        for (uint64_t now = rng.lower(); now < rng.upper(); now += 8)
        {
            printf("[latest] addr: %lx value: %lu\n", now, *(uint64_t *)now);
            latest[now] = *(uint64_t *)now;
        }
    }
    if (rand() % try_cnts-- == 0)
        make_test();
    // }
    //exit
    auto iter = state.begin();
    while (iter != state.end())
    {
        if (iter->second != writeback)
        {
            iter++;
            continue;
        }
        auto lat_left = latest.lower_bound(iter->first.lower());
        auto lat_right = latest.upper_bound(iter->first.upper());
        latest.erase(lat_left, lat_right);
        auto write_log_left = write_log.lower_bound(iter->first.lower());
        auto write_log_right = write_log.upper_bound(iter->first.upper());
        write_log.erase(write_log_left, write_log_right);
        auto k = iter->first;
        iter++;
        state.erase(k);
    }
    printf("[unpersist] latest: %lu state: %lu\n", latest.size(), state.size());
    PIN_ResumeApplicationThreads(tid);
}

void record_final(void *return_ip, uint64_t tid)
{
    printf("[final]\n");
    PIN_StopApplicationThreads(tid);
    for (auto iter = state.begin(); iter != state.end(); iter++)
    {
        auto rng = iter->first;
        for (uint64_t now = rng.lower(); now < rng.upper(); now += 8)
        {
            printf("[latest] addr: %lx value: %lu\n", now, *(uint64_t *)now);
            latest[now] = *(uint64_t *)now;
        }
    }
    make_test();
}

void record_write(addr_t addr, uint64_t size, uint64_t tid)
{
    if (!in_pmem((addr_t)addr))
    {
        return;
    }

    for (uint64_t now = (addr & mask); now < addr + size; now += 8)
    {
        printf("[write] addr: %lx\n", now);
        write_log.insert(std::make_pair(now, *(uint64_t *)now));
    }
    state += std::make_pair(ival::right_open(addr & mask,
                                             (addr + size + 0x7) & mask),
                            (int)pmem_status::modified);
}

void record_pmem_allocation(void *return_ip, uint64_t tid, addr_t addr)
{
    pmem_start = addr;
    pmem_end = addr + 64ll * 1024 * 1024 * 1024;
}

/* ===================================================================== */
// Instrumentation callbacks
/* ===================================================================== */

void FuncMapInit()
{
    std::pair<std::string, TraceFunc *> traceFuncs[] = {
        {"pmem_map_file", &memmap_func},
        // writeback
        {"pmem_flush", &flush_func},
        {"pmem_clflush", &flush_func},
        {"pmem_clflushopt", &flush_func},
        {"pmem_clwb", &flush_func},
        {"flush_clflush", &flush_func},
        {"flush_clflushopt", &flush_func},
        {"flush_clwb", &flush_func},
        // // fence
        {"pmem_drain", &drain_func},
        {"pmem_deep_drain", &drain_func},
        {"trigger_exit", &final_func},
    };

    for (auto pair : traceFuncs)
    {
        FuncMap.insert(pair);
    }
}

void flush_func(RTN rtn)
{
    RTN_InsertCall(
        rtn, IPOINT_BEFORE,
        (AFUNPTR)&record_flush,
        IARG_RETURN_IP,
        IARG_THREAD_ID,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
        IARG_END);
}

void drain_func(RTN rtn)
{
    RTN_InsertCall(
        rtn, IPOINT_BEFORE,
        (AFUNPTR)&record_drain,
        IARG_RETURN_IP,
        IARG_THREAD_ID,
        IARG_END);
}

void final_func(RTN rtn)
{
    RTN_InsertCall(
        rtn, IPOINT_BEFORE,
        (AFUNPTR)&record_final,
        IARG_RETURN_IP,
        IARG_THREAD_ID,
        IARG_END);
}

void memmap_func(RTN rtn)
{
    RTN_InsertCall(
        rtn, IPOINT_AFTER,
        (AFUNPTR)&record_pmem_allocation,
        IARG_RETURN_IP,
        IARG_THREAD_ID,
        IARG_FUNCRET_EXITPOINT_VALUE,
        IARG_END);
}

void PmemFuncsHook(RTN rtn, void *func_map_)
{
    RTN_Open(rtn);
    std::string func_name = RTN_Name(rtn);
    auto pair = FuncMap.find(func_name);
    if (pair != FuncMap.end())
    {
        (*pair->second)(rtn);
    }
    RTN_Close(rtn);
}

void WriteInsHook(INS ins, void *stage)
{
    if (INS_IsMemoryWrite(ins))
    {
        INS_InsertPredicatedCall(
            ins, IPOINT_BEFORE, (AFUNPTR)record_write,
            IARG_MEMORYWRITE_EA,
            IARG_MEMORYWRITE_SIZE,
            IARG_THREAD_ID,
            IARG_END);
    }
}

int main(int argc, char *argv[])
{
    PIN_Init(argc, argv);

    try_cnts = KnobEnableRead.Value();

    PIN_InitSymbols();

    FuncMapInit();

    RTN_AddInstrumentFunction(PmemFuncsHook, nullptr);
    INS_AddInstrumentFunction(WriteInsHook, nullptr);
    PIN_StartProgram();
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
