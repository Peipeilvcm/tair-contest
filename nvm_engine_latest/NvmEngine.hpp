#ifndef TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
#define TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_

#include <atomic>
#include <algorithm>

#include "stdio.h"
#include "include/db.hpp"
#include <vector>
#include <stack>

#include "posix_file_aep.h"
#include "hash.h"
#include "config.h"
#include "spinlock.h"
#include "xxhash.h"

#include "hotkeyhash.h"

// #define LOG_PRINT

const std::string IndexFileName = "-DATAINDEX-";
const std::string DataFileName = "-DATAINDEX-";
const std::string NvmFileName = "-KV";

// typedef SkipList<Index, Compare> Table;

// #define KEY_SIZE 16
// #define DATA_BASE_SIZE 128
// #define MAX_INDEX_SIZE 1073741824


class NvmEngine : DB {
public:
    /**
     * @param 
     * name: file in AEP(exist)
     * dbptr: pointer of db object
     *
     */
    static Status CreateOrOpen(const std::string &name, DB **dbptr);
    NvmEngine(const std::string &name): 
        dbname_(name){
#ifdef LOG_PRINT
        is_first_set = true;
        is_first_get = true;
#endif
        
    }
    Status Get(const Slice &key, std::string *value);
    Status Set(const Slice &key, const Slice &value);
    ~NvmEngine();

    void IndexCallback(uint8_t i);
    void Recover();

    

    inline uint8_t key_to_fileid(char* key){
        // uint32_t cursor = XXH32(key, 16, 0);
        uint32_t cursor = hash(key);

        uint8_t file_index = (cursor & Config::MAX_FILE_COUNT_MOD);
        return file_index;
    }

    // void set_log_file(FILE* log_file){
    //     log_file_ = log_file;
    // }

    // bool is_key_match(char *key, uint32_t offset);

private:

    std::string dbname_;
    

    std::vector<PosixAepFile* > kv_data_files_;
    // MyHashMap* ;
    std::vector<MyHashMap* > hashmap_s;
    
    
    HotKeyPool* hotkeypool;
    

    // FILE* log_file_;

    // std::string dbname_;
    // // Options options_;
    // std::atomic<uint32_t> cursor_;

    // uint32_t max_file_;
    // std::vector<AepFile* > data_files_;
    // std::vector<AepFile* > index_files_;

    // uint32_t table_size_;
    // std::vector<Table *> tables_;

    // std::mutex mutex;

    // std::mutex spl;

#ifdef LOG_PRINT
    bool is_first_set;
    bool is_first_get;
    std::atomic<uint64_t> set_counting;
    std::atomic<uint64_t> get_counting;
    std::chrono::high_resolution_clock::time_point start_time;
#endif
    


};



#endif // TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
