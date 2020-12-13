#include "NvmEngine.hpp"

#include <libpmem.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
// #include <memory.h>
// #include <memory>
#include <unistd.h>

#include <malloc.h>
#include <cassert>

#define USE_LIBPMEM

#include "log.h"
#include "util.h"
#include "likely.h"



Status DB::CreateOrOpen(const std::string &name, DB **dbptr, FILE *log_file) {
    return NvmEngine::CreateOrOpen(name, dbptr);
}

DB::~DB() {}

uint64_t start_time = 0;
Status NvmEngine::CreateOrOpen(const std::string &name, DB **dbptr) {
    log_debug("start time: %lu", NowMicros());
    log_debug("db name: %s", name);
    *dbptr = NULL;

    bool is_new = true;
    if (file_exist((name + "/mapped.data").c_str())) {
      is_new = false;
    }
    start_time = NowMicros();

    NvmEngine *nvm_engine_ptr = new NvmEngine(name, is_new);
    log_debug("[%10lu], start load index, mem usage : %d kb", NowMicros()-start_time, get_memory_usage());
    nvm_engine_ptr->Init();
    log_debug("[%10lu], after load index, mem usage : %d kb", NowMicros()-start_time, get_memory_usage());

    *dbptr = nvm_engine_ptr;

    return Ok;
}



NvmEngine::NvmEngine(const std::string &name) 
    : dir_(name), is_new_(true), first_(true), mappedBufferManager(name + "/mapped.data", true),
    tid_num_(0){
    
    mkdir(name.c_str(), 0755);
    // bucket = new uint32_t[BUCKET_MAX];
    // memset(bucket, 0, sizeof(uint32_t) * BUCKET_MAX);
// #ifdef USE_LIBPMEM
//     if ((entry = (entry_t *)pmem_map_file(name.c_str(),
//                                           ENTRY_MAX * sizeof(entry_t),
//                                           PMEM_FILE_CREATE, 0666, nullptr,
//                                           nullptr)) == NULL) {
//         perror("Pmem map file failed");
//         exit(1);
//     }
// #else
//     if ((entry = (entry_t *)mmap(NULL, BUCKET_SIZE * sizeof(entry_t),
//                                  PROT_READ | PROT_WRITE,
//                                  MAP_ANON | MAP_SHARED, 0, 0)) == NULL) {
//         perror("mmap failed");
//         exit(1);
//     }
// #endif
}

NvmEngine::NvmEngine(const std::string &name, bool is_new) 
    : dir_(name), is_new_(is_new), first_(true), mappedBufferManager(name + "/mapped.data", is_new),
    tid_num_(0){
    
    if(is_new) {
      mkdir(name.c_str(), 0755);
    }
}

Status NvmEngine::Get(const Slice &key, std::string *value) {

    PrintFirstOpTimestamp(false);

    uint64_t key_num = to_uint64(key.data());
    uint32_t seed = (uint32_t) (key_num >> (64 - BUCKET_NUM_BITS));
    value->clear();

    Status status = buckets_[seed].Read(key.data(), value);

    return status;

    // uint32_t hash = _hash(key.data(), key.size());
    // uint32_t slot = hash / BUCKET_PER_MUTEX;
    // slot_mut[slot].lock();
    // for (uint32_t i = 0; i < BUCKET_MAX && bucket[hash]; i++) {
    //     if (memcmp(entry[bucket[hash]].key, key.data(), 16) == 0) {
    //         value->assign(entry[bucket[hash]].value, 80);
    //         slot_mut[slot].unlock();
    //         return Ok;
    //     }

    //     ++hash;
    //     hash %= BUCKET_MAX;
    //     if (hash / BUCKET_PER_MUTEX != slot) {
    //         slot_mut[slot].unlock();
    //         slot = hash / BUCKET_PER_MUTEX;
    //         slot_mut[slot].lock();
    //     }
    // }
    // slot_mut[slot].unlock();
    // return NotFound;
}

Status NvmEngine::Set(const Slice &key, const Slice &value) {

    PrintFirstOpTimestamp(true);

    uint64_t key_num = to_uint64(key.data());

    uint32_t seed = (uint32_t) (key_num >> (64 - BUCKET_NUM_BITS));

    buckets_[seed].Write(key.data(), value.data());

    return Ok;

    // uint32_t hash = _hash(key.data(), key.size());
    // uint32_t slot = hash / BUCKET_PER_MUTEX;
    // slot_mut[slot].lock();
    // uint32_t i;
    // for (i = 0; i < BUCKET_MAX && bucket[hash]; i++) {
    //     if (memcmp(entry[bucket[hash]].key, key.data(), 16) == 0) {
    //         memcpy(entry[bucket[hash]].value, value.data(), 80);
    //         slot_mut[slot].unlock();
    //         return Ok;
    //     }

    //     ++hash;
    //     hash %= BUCKET_MAX;
    //     if (hash / BUCKET_PER_MUTEX != slot) {
    //         slot_mut[slot].unlock();
    //         slot = hash / BUCKET_PER_MUTEX;
    //         slot_mut[slot].lock();
    //     }
    // }
    // if (i == BUCKET_MAX) {
    //     slot_mut[slot].unlock();
    //     return OutOfMemory;
    // }
    // bucket[hash] = entry_cnt.fetch_add(1);
    // memcpy(entry[bucket[hash]].key, key.data(), 16);
    // memcpy(entry[bucket[hash]].value, value.data(), 80);
    // slot_mut[slot].unlock();
    // return Ok;
}

NvmEngine::~NvmEngine() {
    log_debug("end time: %lu", NowMicros());
    log_debug("[%10lu] engine_closed", NowMicros()-start_time);
}

void NvmEngine::PrintFirstOpTimestamp(bool set) {
    if(UNLIKELY(first_)){
        std::unique_lock<std::mutex> lock(mutex_);
        if(first_) {
            first_ = false;
            log_debug("first %s op time: %lu", set ? "set":"get", NowMicros());
        }
    }
}

void NvmEngine::Init(){
    OpenFiles();
    mappedBufferManager.Init();

    for (uint32_t bid = 0; bid < BUCKET_NUM; ++bid) {
      uint32_t gid = GetGroupId(bid);
      buckets_[bid].Init(this, bid, is_new_, key_fd_arr_[gid], value_fd_arr_[gid]);
    }

    std::vector<std::thread> threads;
    const int cpu_num = get_nprocs();

    std::atomic<int> bucket_index(0);
    char* buf_array[cpu_num];

    for (int i = 0; i < cpu_num; ++i) {
        buf_array[i] = (char *) memalign(4096, 512 * 1024);
        threads.emplace_back([this, &bucket_index, buf = buf_array[i]]() {
//      threads.emplace_back([this, &bucket_index]() {

//        char* buf = (char *) memalign(4096, 512 * 1024);
        while (true) {
          int bid = bucket_index++;
          if (bid >= BUCKET_NUM) {
            break;
          }

          buckets_[bid].LoadIndex(buf);

        }
//        free(buf);
      });
    }

    for (int i = 0; i < cpu_num; ++i) {
      threads[i].join();
      free(buf_array[i]);
    }
}

void NvmEngine::OpenFiles(){
    assert(BUCKET_NUM%GROUP_NUM == 0);
    int imode = 0;
    if (is_new_) {
      imode |= USE_DIRECT|O_NOATIME | O_RDWR | O_CREAT;
    } else {
      imode |= USE_DIRECT|O_NOATIME | O_RDONLY | O_CREAT;
    }
    for (int i = 0; i < GROUP_NUM; ++i) {
        std::string keyDataFileName = dir_ + "/key" + std::to_string(i) + ".data";
        std::string valueDataFileName = dir_ + "/value" + std::to_string(i) + ".data";

        log_debug("keyDataFileName: %s", keyDataFileName.c_str());
        key_fd_arr_[i] = open(keyDataFileName.c_str(), imode, 0644);
        value_fd_arr_[i] = open(valueDataFileName.c_str(), imode, 0644);
        if (key_fd_arr_[i] < 0 || value_fd_arr_[i] < 0) {
            log_error("open file failed: %s", strerror(errno));
            exit(-1);
        }
        if (is_new_) {
            if(ftruncate64(value_fd_arr_[i], BUCKET_NUM_IN_GROUP * BUCKET_VALUE_MAX_SIZE) != 0) {
                log_error("file truncate failed: %s", strerror(errno));
                exit(-1);
            }
            if(ftruncate64(key_fd_arr_[i], BUCKET_NUM_IN_GROUP * BUCKET_KEY_MAX_SIZE) != 0) {
                log_error("file truncate failed: %s", strerror(errno));
                exit(-1);
            }
            int num = i+1;
//        pwrite64(value_fd_arr_[i], (char*)&num, 4096, 0);
//        pwrite64(key_fd_arr_[i], (char*)&num, 4096, 0);

        }
    }
}


