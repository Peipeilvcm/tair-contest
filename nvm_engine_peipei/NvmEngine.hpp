#ifndef TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
#define TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_

#include <atomic>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <mutex>

#include "include/db.hpp"

#include <unordered_map>
#include <map>
#include <sys/time.h>
#include <thread>

#include "Bucket.h"
#include "mapped_buffer_manager.h"
#include "config.h"
#include "util.h"



class NvmEngine : DB {
public:
    /**
     * @param 
     * name: file in AEP(exist)
     * dbptr: pointer of db object
     *
     */
    static Status CreateOrOpen(const std::string &name, DB **dbptr);
    NvmEngine(const std::string &name);
    Status Get(const Slice &key, std::string *value);
    Status Set(const Slice &key, const Slice &value);
    ~NvmEngine();

    friend class Bucket;
    NvmEngine(const std::string &name, bool is_new);


private:

    std::string dir_;
    bool is_new_;
    bool first_;

    std::mutex mutex_;

    Bucket buckets_[BUCKET_NUM];
    MappedBufferManager mappedBufferManager;

    std::atomic<int> tid_num_;

    int value_fd_arr_[GROUP_NUM];
    int key_fd_arr_[GROUP_NUM];

    void Init();
    void OpenFiles();
    void PrintFirstOpTimestamp(bool set);

    // static uint32_t _hash(const char *str, uint16_t size);
    // static const size_t NVM_SIZE = 79456894976;
    // static const size_t DRAM_SIZE = 4200000000;
    // struct entry_t {
    //     char key[16];
    //     char value[80];
    // };
    // static const uint32_t ENTRY_MAX = NVM_SIZE / sizeof(entry_t);
    // static const uint32_t BUCKET_MAX = DRAM_SIZE / sizeof(uint32_t);
    // static const uint32_t BUCKET_PER_MUTEX = 30000000;
    // static const uint32_t MUTEX_CNT = BUCKET_MAX / BUCKET_PER_MUTEX + 1;

    // entry_t *entry;
    // uint32_t *bucket;
    // std::atomic<uint32_t> entry_cnt = {1};
    // std::mutex slot_mut[MUTEX_CNT];
};



#endif
