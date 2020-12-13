#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <cstring>

#include "posix_file_aep.h"

#include "config.h"

#include <libpmem.h>

PosixAepFile::~PosixAepFile() {
    for (auto st :  free_space) {
        delete st;
    }
    this->Close();
}


void PosixAepFile::ReadKV_Data(uint32_t offset, uint16_t *value_size_, char* value){
    memcpy(value_size_, fileptr_+offset+Config::KEY_SIZE, sizeof(*value_size_));
    memcpy(value, fileptr_+offset+Config::KEY_SIZE+4, *value_size_);
}

void PosixAepFile::ReadKV_KVs(uint32_t offset, char* key, uint16_t *value_size_, uint8_t *key_set_cnts, uint8_t *chunk_id){
    memcpy(key, fileptr_+offset, Config::KEY_SIZE);
    memcpy(value_size_, fileptr_+offset+Config::KEY_SIZE, sizeof(*value_size_));
    memcpy(key_set_cnts, fileptr_+offset+Config::KEY_SIZE+2, sizeof(uint8_t));
    memcpy(chunk_id, fileptr_+offset+Config::KEY_SIZE+3, sizeof(uint8_t));
}

void PosixAepFile::ReadKV_SetCnts(uint32_t offset, uint8_t *key_set_cnts){
    memcpy(key_set_cnts, fileptr_+offset+Config::KEY_SIZE+2, sizeof(uint8_t));
}

bool PosixAepFile::CMP_KEY(uint32_t offset, char *key){

    int ret = memcmp(key, fileptr_+offset, Config::KEY_SIZE);
    if(ret == 0){
        return true;
    }
    return false;
}

void PosixAepFile::InsertKV_Data(const Slice &key, const Slice &value, uint8_t key_set_cnt, uint32_t *value_head_offset){

    uint16_t item_size;
    int idx = calFreeSpaceIdx(value.size(), &item_size);

    spl.lock();
    for(uint8_t i = idx; i < free_space.size(); ++i){
        if(!free_space[i]->empty()){
            *value_head_offset = free_space[i]->front();
            free_space[i]->pop();
            spl.unlock();

            SetKV_Data(*value_head_offset, key, value, key_set_cnt);
            return;
        }
    }

    // 开辟新空间
    *value_head_offset = file_end_offset_;
    file_end_offset_.fetch_add(item_size);
    CreateKV_Data(*value_head_offset, key, value, key_set_cnt);
    spl.unlock();

    return;
}

void PosixAepFile::RecycleKV_Data(uint32_t old_index_offset){// 回收内存
    // uint16_t old_value_size;
    // memcpy(&old_value_size, fileptr_+old_index_offset+Config::KEY_SIZE, sizeof(old_value_size));

    // int idx = calFreeSpaceIdx(old_value_size, nullptr);
    uint8_t idx;
    memcpy(&idx, fileptr_+old_index_offset+Config::KEY_SIZE+3, sizeof(uint8_t));

    spl.lock();
    free_space[idx]->push(old_index_offset);
    spl.unlock();
}

void PosixAepFile::UpdateKV_Data(const Slice &key, const Slice &value, uint32_t* value_head_offset, uint32_t old_index_offset, uint8_t old_key_set_cnt){
    
    InsertKV_Data(key, value, old_key_set_cnt + 1, value_head_offset);

    RecycleKV_Data(old_index_offset);
}

void PosixAepFile::SetKV_Data(uint32_t offset, const Slice &key, const Slice &value, uint8_t key_set_cnt){

    uint16_t value_size = value.size();
    
    memcpy(fileptr_ +  offset, key.data(), Config::KEY_SIZE);//Key
    memcpy(fileptr_ +  offset + Config::KEY_SIZE, &value_size, 2);//valuesize
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 2, &key_set_cnt, sizeof(uint8_t));//keysetcnt
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 4, value.data(), value_size);//value
    pmem_persist(fileptr_ + offset , Config::KEY_SIZE + 4 + value_size);
}

void PosixAepFile::CreateKV_Data(uint32_t offset, const Slice &key, const Slice &value, uint8_t key_set_cnt){

    uint16_t value_size = value.size();
    uint8_t idx = calFreeSpaceIdx(value_size, nullptr);
    
    memcpy(fileptr_ +  offset, key.data(), Config::KEY_SIZE);//Key
    memcpy(fileptr_ +  offset + Config::KEY_SIZE, &value_size, 2);//valuesize
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 2, &key_set_cnt, sizeof(uint8_t));//keysetcnt
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 3, &idx,  sizeof(uint8_t));// chunksize
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 4, value.data(), value_size);//value
    pmem_persist(fileptr_ + offset , Config::KEY_SIZE + 4 + value_size);
}

void PosixAepFile::Close() {
}
