// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_HOT_H_
#define TAIR_CONTEST_KV_CONTEST_HOT_H_

#include <stdint.h>
#include <string.h>
#include "config.h"

// #include "posix_file_aep.h"
#include "spinlock.h"
#include "xxhash.h"


#include "include/db.hpp"


struct key_map{
    uint32_t offset;
    char key[Config::KEY_SIZE];
};

class KeyMap
{
private:
    // static const uint32_t ADDRESS_SIZE = 2147483648;// 67108864; // 1024/16
    
    // std::atomic<uint64_t> set_counting;
    // spinlock spl;
    key_map key_arrays[Config::KEY_MAP_SIZE];
    // char address[ADDRESS_SIZE];
public:

    static uint32_t keyhash(uint32_t offset)
    {
        // uint32_t hash = *((uint32_t*)str);
        // uint32_t hash = XXH64(str, 16, 0);
        
        return offset % Config::KEY_MAP_SIZE;
    }

    /** Initialize your data structure here. */
    KeyMap()
    {
        memset(key_arrays, 0xFF, sizeof(key_arrays));
    }

    ~KeyMap() {}

    /** value will always be non-negative. */

    void setKV(const uint32_t& offset, char* key, uint32_t key_pos_, bool is_updated = false){

        if(!is_updated){
            key_arrays[key_pos_].offset = offset;
            // memcpy(key_index_offsets[key_pos_].key, key.data(), Config::KEY_SIZE);
        }

        memcpy(key_arrays[key_pos_].key, key, Config::KEY_SIZE);
    }

    bool getKV(const uint32_t& offset, char* key, uint32_t *key_pos_){
        // uint32_t key_pos;
        bool success = get_k(offset, key_pos_);

        if(success){
            memcpy(key, key_arrays[*key_pos_].key, Config::KEY_SIZE);
            // *value = std::string(key_index_offsets[*key_pos_].value, key_index_offsets[*key_pos_].value_size);
            return true;
        }else{
            return false;
        }
    }

    bool get_k(const uint32_t& offset, uint32_t *key_pos){

        uint32_t k = keyhash(offset);

        while(key_arrays[k].offset != 0xFFFFFFFF){

            if(key_arrays[k].offset == offset){
                *key_pos = k;
                return true;
            }

            if( ++k == Config::KEY_MAP_SIZE){
                k = 0;
            }
        }

        *key_pos = k;
        return false;

        // *key_pos = 

        
        // if(key_index_offsets[*key_pos].value_size == 0){
        //     return false;
        // }else{
        //     int ret = -1;
        //     ret = memcmp(key, key_index_offsets[*key_pos].key, Config::KEY_SIZE); // 命中
        //     if(ret == 0){
        //         return true;
        //     }
        //     return false; // 冲突
        // }
    }

};

#endif // TAIR_CONTEST_KV_CONTEST_HOT_H_
