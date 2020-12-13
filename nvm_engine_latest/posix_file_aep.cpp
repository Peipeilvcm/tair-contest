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
    // for (auto splp : free_space_spls){
    //     delete splp;
    // }
    this->Close();
}


// keysetcnt4 + Key16 + valuesize2 + chunksize1 + value + keysetcnt4
void PosixAepFile::ReadKV_Data(uint32_t offset, uint16_t *value_size_, char* value){

    memcpy(value_size_, fileptr_+offset+Config::KEY_SIZE+4, 2);
    memcpy(value, fileptr_+offset+Config::KEY_SIZE+7, *value_size_);
}


void PosixAepFile::ReadKV_KVs(uint32_t offset, char* key, uint16_t *value_size_, uint32_t *key_set_cnts, uint8_t *chunk_id){
    
    memcpy(key, fileptr_+offset+4, Config::KEY_SIZE);
    memcpy(value_size_, fileptr_+offset+Config::KEY_SIZE+4, sizeof(*value_size_));
    memcpy(chunk_id, fileptr_+offset+Config::KEY_SIZE+6, sizeof(uint8_t));

    int ret = memcmp(fileptr_+offset, fileptr_+offset+calSizeFromIdx(*chunk_id)-4, 4);
    if(ret == 0){
        memcpy(key_set_cnts, fileptr_+offset+calSizeFromIdx(*chunk_id)-4, sizeof(uint32_t));
    }else{
         *key_set_cnts = 0xFFFFFFFF;
    }
    
    // memcpy(chunk_id, fileptr_+offset+Config::KEY_SIZE+3, sizeof(uint8_t));
}

void PosixAepFile::ReadKV_SetCnts(uint32_t offset, uint32_t *key_set_cnts){
    uint8_t chunk_id;
    memcpy(&chunk_id, fileptr_+offset+Config::KEY_SIZE+6, sizeof(uint8_t));
    memcpy(key_set_cnts, fileptr_+offset+calSizeFromIdx(chunk_id)-4, sizeof(uint32_t));
}

bool PosixAepFile::CMP_KEY(uint32_t offset, char *key){
    int ret = memcmp(key, fileptr_+offset+4, Config::KEY_SIZE);
    if(ret == 0){
        return true;
    }
    return false;
}


void PosixAepFile::AppendKV_Data(const Slice &key, const Slice &value, uint32_t* index_head_offset){ //新key必须append
    uint16_t item_size;
    int idx = calFreeSpaceIdx(value.size(), &item_size);

    // fprintf(stderr, "Key=%s, AppendKV Set index_offset\n", key.to_string().c_str());

    
    key_set_cnts_f_spl.lock();
    // fprintf(stderr, "B\n", key.to_string().c_str());
    this->key_set_cnts_f++;
    uint32_t key_set_cnt = this->key_set_cnts_f;
    key_set_cnts_f_spl.unlock();

    // findUsefulSpace(idx, index_head_offset);
    
    uint8_t free_space_size = free_space.size();
    uint8_t sup_space = idx + 15;
    // uint32_t key_set_cnts, index_offset;
    // spl.lock();
    for(uint8_t i = idx; i < free_space_size && i < sup_space; ++i){
        // fprintf(stderr, "C\n");
        
        // fprintf(stderr, "D\n");
        if(free_space[i]->size() > 15){
            free_space_spls[i].lock();
            *index_head_offset = free_space[i]->top();
            free_space[i]->pop();
            free_space_spls[i].unlock();
            // spl.unlock();

            SetKV_Data(*index_head_offset, key, value, key_set_cnt);
            return;
        }
        // free_space_spls[i].unlock();
    }
    // spl.unlock();
    // fprintf(stderr, "Key=%s, AppendKV Set index_offset offset = %llu\n", key.to_string().c_str(), *index_head_offset);

    // 开辟新空间
    // spl.lock();
    file_end_offset_spl.lock();
    *index_head_offset = file_end_offset_;
    file_end_offset_ += item_size;
    file_end_offset_spl.unlock();
    
    CreateKV_Data(*index_head_offset, key, value, key_set_cnt);
}

void PosixAepFile::UpdateKV_Data(const Slice &key, const Slice &value, uint32_t* value_head_offset, const uint32_t& old_index_offset, const uint32_t& old_key_set_cnts){

    // InsertKV_Data(key, value, value_head_offset);
    uint16_t item_size;
    int idx = calFreeSpaceIdx(value.size(), &item_size);


    
    key_set_cnts_f_spl.lock();
    this->key_set_cnts_f++;
    uint32_t key_set_cnt = this->key_set_cnts_f;
    key_set_cnts_f_spl.unlock();
    // spl.unlock();

    
    uint8_t sup_space = idx + 15;
    uint8_t free_space_size = free_space.size();
    // uint32_t key_set_cnts, index_offset;
    // spl.lock();
    for(uint8_t i = idx; i < free_space_size && i < sup_space; ++i){
        // fprintf(stderr, "C\n");
        // fprintf(stderr, "D\n");
        if(free_space[i]->size() > 15){
            free_space_spls[i].lock();
            *value_head_offset = free_space[i]->top();
            free_space[i]->pop();
            free_space_spls[i].unlock();
            // spl.unlock();

            SetKV_Data(*value_head_offset, key, value, key_set_cnt);

            RecycleKV_Data(old_index_offset);
            return;
        }
        // free_space_spls[i].unlock();
    }
    // spl.unlock();

    // fprintf(stderr, "Key=%s, AppendKV Set index_offset offset = %llu\n", key.to_string().c_str(), *index_head_offset);

    // 开辟新空间
    // spl.lock();
    file_end_offset_spl.lock();
    *value_head_offset = file_end_offset_;
    file_end_offset_ += item_size;
    file_end_offset_spl.unlock();
    // spl.unlock();
    CreateKV_Data(*value_head_offset, key, value, key_set_cnt);

    RecycleKV_Data(old_index_offset);
}


// keysetcnt4 + Key16 + valuesize2 + chunksize1 + value + keysetcnt4
void PosixAepFile::RecycleKV_Data(const uint32_t& old_index_offset){// 回收内存
    
    uint8_t idx;
    memcpy(&idx, fileptr_+old_index_offset+Config::KEY_SIZE+6, 1);
    
    // std::pair<uint32_t, uint32_t> fs(old_key_set_cnts, old_index_offset);

    // spl.lock();
    if(free_space[idx]->size() < Config::MAX_FREE_SPACE_SIZE){
        free_space_spls[idx].lock();
        free_space[idx]->push(old_index_offset);
        free_space_spls[idx].unlock();
    }
    // spl.unlock();
}



// keysetcnt4 + Key16 + valuesize2 + chunksize1 + value + keysetcnt4
void PosixAepFile::SetKV_Data(uint32_t offset, const Slice &key, const Slice &value, const uint32_t& key_set_cnt){

    memcpy(fileptr_ +  offset, &key_set_cnt, 4);//keysetcnt

    uint16_t value_size = value.size();

    memcpy(fileptr_ +  offset + 4, key.data(), Config::KEY_SIZE);//Key
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 4, &value_size, 2);//valuesize

    uint8_t chunk_id;
    memcpy(&chunk_id, fileptr_+offset+Config::KEY_SIZE+6, sizeof(uint8_t)); //chunksize
    uint16_t item_size = calSizeFromIdx(chunk_id);

    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 7, value.data(), value_size);//value
    memcpy(fileptr_ +  offset + item_size - 4, &key_set_cnt, sizeof(uint32_t));//keysetcnt
    
    pmem_persist(fileptr_ + offset , item_size);
}

// keysetcnt4 + Key16 + valuesize2 + chunksize1 + value + keysetcnt4
void PosixAepFile::CreateKV_Data(uint32_t offset, const Slice &key, const Slice &value, const uint32_t& key_set_cnt){

    uint16_t value_size = value.size();
    uint16_t item_size;
    uint8_t idx = calFreeSpaceIdx(value_size, &item_size);
    
    memcpy(fileptr_ +  offset, &key_set_cnt, 4);//keysetcnt
    memcpy(fileptr_ +  offset + 4, key.data(), Config::KEY_SIZE);//Key
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 4, &value_size, 2);//valuesize
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 6, &idx,  sizeof(uint8_t));// chunksize
    memcpy(fileptr_ +  offset + Config::KEY_SIZE + 7, value.data(), value_size);//value
    memcpy(fileptr_ +  offset + item_size - 4, &key_set_cnt, sizeof(uint32_t));//keysetcnt

    pmem_persist(fileptr_ + offset , item_size);
}

void PosixAepFile::Close() {
}
