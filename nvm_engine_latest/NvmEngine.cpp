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

    // Open Hot Key
    nvm_engine_ptr->hotkeypool = new HotKeyPool();

    fprintf(Config::global_log_ptr, "Hot key Pool successfully\n");
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

}

Status NvmEngine::Set(const Slice &key, const Slice &value) {


    // // spl.lock();
    uint32_t key_pos_hot = 0;
    if(hotkeypool->get_k(key.data(), &key_pos_hot)){
        hotkeypool->setKV(key, value.data(),  value.size(), key_pos_hot, true);//更新key值
    }


    uint8_t file_index = key_to_fileid(key.data());

    MyHashMap* hashmap_ = hashmap_s[file_index];

    PosixAepFile* kv_data_file = kv_data_files_[file_index];

    // fprintf(stderr, "Set file_index %u\n", file_index);

    // 要先get 再判断是不是要更新
    uint32_t hash_pos = 0;
    uint32_t key_set_cnt = 0;
    
    uint32_t index_offset = hashmap_->get(key.data(), kv_data_file, &hash_pos, &key_set_cnt);
    // spl.unlock();
    
    // uint32_t reoffset;
    if(index_offset == Config::INDEX_OFFSET_EMPTY){ // 新增
        // fprintf(stderr, "Set Add\n");
        // 新增
        kv_data_file->AppendKV_Data(key, value, &index_offset);
        hashmap_->put(key.data(), index_offset);
 
        // reoffset = index_offset;
        
    }else{
        // Update
        uint32_t new_index_offset;
        kv_data_file->UpdateKV_Data(key, value, &new_index_offset, index_offset, key_set_cnt);
        hashmap_->update(hash_pos, new_index_offset);

        // reoffset = new_index_offset;
    }

    // this->set_counting++;
    // if(is_first_set){
    //     is_first_set=false;
    //     fprintf(Config::global_log_ptr, "First Set Time %s\n", getTime().c_str());
    //     fflush(Config::global_log_ptr);
    // }
    // if((uint64_t)this->set_counting % 1000000 == 0){
    //     fprintf(Config::global_log_ptr, "Set Time %s, set counting=%lld, fileid=%u, indexoffset=%lu, value_size=%u\n", getTime().c_str(), (uint64_t) this->set_counting, file_index, reoffset, value.size());
    //     fflush(Config::global_log_ptr);
    // }
#ifdef LOG_PRINT
this->set_counting++;
if(is_first_set){
    is_first_set=false;
    start_time = std::chrono::high_resolution_clock::now();
}
if((uint64_t)this->set_counting % 1000000 == 0){
    std::chrono::high_resolution_clock::time_point now_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now_time - start_time);
    fprintf(Config::global_log_ptr, "Set:::: set counting=%llu, time cost=%lf\n",  (uint64_t) this->set_counting, time_span.count());
    fflush(Config::global_log_ptr);
}
#endif

    return Ok;
}

Status NvmEngine::Get(const Slice &key, std::string *value) {
    // spl.lock();
    // fprintf(stderr, "Geta\n");
    uint32_t key_pos_hot = 0;
    if(hotkeypool->getKV(key, value, &key_pos_hot)){
        // fprintf(stderr, "OK\n");
        return Ok;
    }
    // fprintf(stderr, "hot key cnt file_index %u, kpos=%llu\n", key_cnts_hot, key_pos_hot);

    uint8_t file_index = key_to_fileid(key.data());
    PosixAepFile* kv_data_file = kv_data_files_[file_index];

    // spl.lock();
    uint32_t index_offset = hashmap_s[file_index]->get(key.data(), kv_data_file);
    
    

#ifdef LOG_PRINT
this->get_counting++;
if((uint64_t)this->get_counting % 1000000 == 0){
    std::chrono::high_resolution_clock::time_point now_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(now_time - start_time);
    fprintf(Config::global_log_ptr, "Get:::: get counting=%llu, time cost=%lf\n", (uint64_t) this->get_counting, time_span.count());
    fflush(Config::global_log_ptr);
}
if((uint64_t)this->get_counting > 12100000){
    int a[2];
    int c = a[20];
    exit(134);
}
#endif


    // if(is_first_get && (uint64_t)this->get_counting > 100000){
    //     is_first_get=false;
    //     fprintf(Config::global_log_ptr, "First Get Time %s, set counting=%lld\n", getTime().c_str(), (uint64_t) this->set_counting);
    //     fflush(Config::global_log_ptr);
    // }
    // if((uint64_t)this->get_counting % 1000000 == 1){
    //     fprintf(Config::global_log_ptr, "Get Time %s, get counting=%lld\n", getTime().c_str(), (uint64_t) this->get_counting);
    //     fflush(Config::global_log_ptr);
    // }


    if(index_offset == Config::INDEX_OFFSET_EMPTY){
        fprintf(Config::global_log_ptr, "KEY=%s, NotFound\n", key.to_string().c_str());
        fflush(Config::global_log_ptr);
        return NotFound;
    }

    // fprintf(stderr, "Key = %s, Get index_offset %u\n", key.to_string().c_str(), index_offset);

    char buf_value[1024];
    uint16_t value_size;
    kv_data_file->ReadKV_Data(index_offset, &value_size, buf_value);

    *value = std::string(buf_value, value_size);

    hotkeypool->setKV(key, buf_value, value_size, key_pos_hot, false);

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
    uint32_t key_set_cnt = 0;
    uint8_t chunk_id = 0;
    kv_data_file->ReadKV_KVs(pos, buf_key, &value_size, &key_set_cnt, &chunk_id);

    // printf("Starting Recover, FILE_COUNT=%u, %u\n", i, value_size);

    uint32_t hash_pos=0;
    uint32_t old_key_set_cnt=0;
    uint32_t old_index_offset=0;

    uint32_t max_set_cnt = 0;

    while(value_size && key_set_cnt){

        if(key_set_cnt == 0xFFFFFFFF){
            // 回收当前空间
            kv_data_file->RecycleKV_Data(pos);
        }else{
            if(key_set_cnt > max_set_cnt){
                max_set_cnt = key_set_cnt;
            }
            
            old_index_offset = hashmap_->get(buf_key, kv_data_file, &hash_pos, &old_key_set_cnt);

            if(old_index_offset == Config::INDEX_OFFSET_EMPTY){//新增
                hashmap_->put(buf_key, pos);
            }else{// 更新
                if(key_set_cnt > old_key_set_cnt){
                    hashmap_->update(hash_pos, pos);
                    // 回收旧空间
                    kv_data_file->RecycleKV_Data(old_index_offset);
                }else{
                    // 回收当前空间
                    kv_data_file->RecycleKV_Data(pos);
                }
            }
        }

        uint16_t item_size = PosixAepFile::calSizeFromIdx(chunk_id);
        pos += item_size;
        kv_data_file->ReadKV_KVs(pos, buf_key, &value_size, &key_set_cnt, &chunk_id);
    }
    kv_data_file->setFileEndOffset(pos);
    kv_data_file->setKeySetCnts(max_set_cnt);
}


