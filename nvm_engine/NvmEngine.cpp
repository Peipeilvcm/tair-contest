#include "NvmEngine.hpp"
#include <sys/stat.h>


#define USE_LIBPMEM

#include <iostream>
#include <thread>
#include <string.h>

#include "env.h"
#include "hash.h"

#include "time.h"

FILE* Config::global_log_ptr;

Status DB::CreateOrOpen(const std::string &name, DB **dbptr, FILE *log_file) {
    Config::global_log_ptr = log_file;
    return NvmEngine::CreateOrOpen(name, dbptr);
}

DB::~DB() {}

std::string getTime()
{
    time_t timep;
    time (&timep); //获取time_t类型的当前时间
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );//对日期和时间进行格式化
    return tmp;
}


Status NvmEngine::CreateOrOpen(const std::string &name, DB **dbptr) {
    // fprintf(stderr, "opening database\n");
    fprintf(Config::global_log_ptr, "opening database\n");
    fflush(Config::global_log_ptr);

    *dbptr = nullptr;

    NvmEngine *nvm_engine_ptr = new NvmEngine(name);

    // nvm_engine_ptr->set_log_file(log_file);

    std::string aep_file_name = name + NvmFileName;
    char *flieptr = nullptr;
    // size_t mapped_len;
    // int is_pmem;
    if ((flieptr = (char *)pmem_map_file(aep_file_name.c_str(), Config::TOTAL_PMEM_MAX_SIZE, PMEM_FILE_CREATE, 0666, nullptr, nullptr)) == NULL)
    {
        fprintf(Config::global_log_ptr, "pmem_map_file\n");
        perror("pmem_map_file");
        exit(1);
    }

    // create offset hashtable
    for (uint8_t i = 0; i < Config::MAX_FILE_COUNT; i++) {
        MyHashMap* hmp = new MyHashMap();
        nvm_engine_ptr->hashmap_s.push_back(hmp);
    }

    // Open key value data
    for (uint8_t i = 0; i < Config::MAX_FILE_COUNT; i++) {
        PosixAepFile* file;
        Env::NewKVFile(aep_file_name, flieptr, i, &file);
        nvm_engine_ptr->kv_data_files_.push_back(file);
    }

    fprintf(Config::global_log_ptr, "Opening kv files successfully\n");
    fflush(Config::global_log_ptr);


    nvm_engine_ptr->Recover();

    fprintf(Config::global_log_ptr, "Recover successfully\n");
    fflush(Config::global_log_ptr);
    
    *dbptr = nvm_engine_ptr;

    return Ok;
}





NvmEngine::~NvmEngine() {
    fprintf(Config::global_log_ptr, "closing database\n");
    fflush(Config::global_log_ptr);

    for (auto file : kv_data_files_) {
        delete file;
    }

    for (auto hmp : hashmap_s) {
        delete hmp;
    }

    // delete hashmap_;
    // for (auto table: tables_) {
    //     delete table;
    // }
}

Status NvmEngine::Set(const Slice &key, const Slice &value) {
    // spl.lock();

    uint8_t file_index = key_to_fileid(key.data());

    MyHashMap* hashmap_ = hashmap_s[file_index];

    PosixAepFile* kv_data_file = kv_data_files_[file_index];

    // fprintf(stderr, "Set file_index %u\n", file_index);

    // 要先get 再判断是不是要更新
    // uint16_t old_value_size;
    uint32_t hash_pos = 0;
    uint8_t key_set_cnt = 0;
    // spl.lock();
    uint32_t index_offset = hashmap_->get(key.data(), kv_data_file, &hash_pos, &key_set_cnt);
    // spl.unlock();
    

    // fprintf(stderr, "Set index_offset %u\n", index_offset);
    // uint32_t reoffset;

    if(index_offset == Config::INDEX_OFFSET_EMPTY){ // 新增

        // fprintf(stderr, "Set Add\n");
        // 新增
        // std::lock_guard<std::mutex> lock(mutex);
        // spl.lock();
        kv_data_file->InsertKV_Data(key, value,  1, &index_offset);
        // spl.unlock();

        // spl.lock();
        hashmap_->put(key.data(), index_offset);
        // spl.unlock();

        // spl.unlock();
        // reoffset = index_offset;
        
    }else{
        // Update
        uint32_t new_index_offset;

        // spl.lock();
        kv_data_file->UpdateKV_Data(key, value, &new_index_offset, index_offset, key_set_cnt);
        // spl.unlock();

        // spl.lock();
        hashmap_->update(hash_pos, new_index_offset);
        // spl.unlock();
        
        // spl.unlock();
        // reoffset = new_index_offset;
    }

    // this->set_counting++;
    // if(is_first_set){
    //     is_first_set=false;
    //     fprintf(Config::global_log_ptr, "First Set Time %s\n", getTime().c_str());
    //     fflush(Config::global_log_ptr);
    // }
    // if((uint64_t)this->set_counting % 1000000 == 1){
    //     fprintf(Config::global_log_ptr, "Set Time %s, set counting=%lld, fileid=%u, indexoffset=%lu, value_size=%u\n", getTime().c_str(), (uint64_t) this->set_counting, file_index, reoffset, value.size());
    //     fflush(Config::global_log_ptr);
    // }

    return Ok;
}

Status NvmEngine::Get(const Slice &key, std::string *value) {
    // spl.lock();

    // this->get_counting++;
    // if(is_first_get && (uint64_t)this->get_counting > 100000){
    //     is_first_get=false;
    //     fprintf(Config::global_log_ptr, "First Get Time %s, set counting=%lld\n", getTime().c_str(), (uint64_t) this->set_counting);
    //     fflush(Config::global_log_ptr);
    // }
    // if((uint64_t)this->get_counting % 1000000 == 1){
    //     fprintf(Config::global_log_ptr, "Get Time %s, get counting=%lld\n", getTime().c_str(), (uint64_t) this->get_counting);
    //     fflush(Config::global_log_ptr);
    // }


    uint8_t file_index = key_to_fileid(key.data());
    PosixAepFile* kv_data_file = kv_data_files_[file_index];

    uint32_t index_offset = hashmap_s[file_index]->get(key.data(), kv_data_file);


    if(index_offset == Config::INDEX_OFFSET_EMPTY){
        fprintf(Config::global_log_ptr, "KEY=%s, NotFound\n", key.to_string().c_str());
        fflush(Config::global_log_ptr);
        return NotFound;
    }

    // fprintf(stderr, "A\n");

    char buf_value[1024];
    uint16_t value_size;
    kv_data_file->ReadKV_Data(index_offset, &value_size, buf_value);

    *value = std::string(buf_value, value_size);

    // spl.unlock();

    return Ok;

}


void NvmEngine::Recover() {

    fprintf(Config::global_log_ptr, "Recover Start Time %s\n", getTime().c_str());

    std::vector<std::thread > threads;
    for (uint8_t i = 0; i < Config::MAX_FILE_COUNT; i++) {
        std::thread recover(&NvmEngine::IndexCallback, this, i);
        threads.push_back(std::move(recover));
    }

    for (uint8_t i = 0; i < Config::MAX_FILE_COUNT; i++) {
        threads[i].join();
    }

    fprintf(Config::global_log_ptr, "Recover End Time %s\n", getTime().c_str());
}



void NvmEngine::IndexCallback(uint8_t i) {

    PosixAepFile* kv_data_file = kv_data_files_[i];

    MyHashMap* hashmap_ = hashmap_s[i];

    uint32_t pos = 0;
    char buf_key[Config::KEY_SIZE];
    uint16_t value_size;
    uint8_t key_set_cnt = 0;
    uint8_t chunk_id = 0;
    kv_data_file->ReadKV_KVs(pos, buf_key, &value_size, &key_set_cnt, &chunk_id);

    // printf("Starting Recover, FILE_COUNT=%u, %u\n", i, value_size);

    uint32_t hash_pos=0;
    uint8_t old_key_set_cnt=0;
    uint32_t old_index_offset=0;

    while(value_size){

        old_index_offset = hashmap_->get(buf_key, kv_data_files_[i], &hash_pos, &old_key_set_cnt);

        if(old_index_offset == Config::INDEX_OFFSET_EMPTY){//新增
            hashmap_->put(buf_key, pos);
        }else{// 更新
            if(key_set_cnt > old_key_set_cnt){
                hashmap_->update(hash_pos, pos);
                // 回收旧空间
                kv_data_files_[i]->RecycleKV_Data(old_index_offset);
            }else{
                // 回收当前空间
                kv_data_files_[i]->RecycleKV_Data(pos);
            }
        }

        uint16_t item_size = PosixAepFile::calSizeFromIdx(chunk_id);
        pos += item_size;
        // printf("pos, FILE_COUNT=%lu\n", pos);
        kv_data_file->ReadKV_KVs(pos, buf_key, &value_size, &key_set_cnt, &chunk_id);
        // printf("Starting Recover, FILE_COUNT=%u, %u\n", i, value_size);
    }
    kv_data_file->setFileEndOffset(pos);
}


