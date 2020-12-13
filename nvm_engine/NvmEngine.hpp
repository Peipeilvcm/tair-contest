#ifndef TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
#define TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_

#include <atomic>
#include <algorithm>

#include "stdio.h"
#include "include/db.hpp"
#include <vector>
#include <stack>

#include "index.h"
#include "posix_file_aep.h"
#include "hash.h"
#include "config.h"
#include "spinlock.h"


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
        // is_first_set = true;
        // is_first_get = true;
    }
    Status Get(const Slice &key, std::string *value);
    Status Set(const Slice &key, const Slice &value);
    ~NvmEngine();

    void IndexCallback(uint8_t i);
    void Recover();

    

    inline uint8_t key_to_fileid(char* key){
        uint32_t cursor = *((uint32_t*)key);
        uint8_t file_index = (cursor & (Config::MAX_FILE_COUNT - 1));
        return file_index;
    }

    // void set_log_file(FILE* log_file){
    //     log_file_ = log_file;
    // }

    // bool is_key_match(char *key, uint32_t offset);

private:

    std::string dbname_;
    

    char* index_fileptr_;

    // std::vector<PosixAepFile* > data_files_;
    // std::vector<PosixAepFile* > index_files_;
    std::vector<PosixAepFile* > kv_data_files_;
    // MyHashMap* ;
    std::vector<MyHashMap* > hashmap_s;
    
    

    

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

    spinlock spl;

    // bool is_first_set;
    // bool is_first_get;
    // std::atomic<uint64_t> set_counting;
    // std::atomic<uint64_t> get_counting;
};



#endif // TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
