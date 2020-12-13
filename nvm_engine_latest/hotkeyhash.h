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


struct hot_key_index{
    char key[Config::KEY_SIZE];
    // uint32_t value_offset;
    char value[1024];
    uint16_t value_size;
};


// to do cockoo hash
class HotKeyPool
{
private:
    // static const uint32_t ADDRESS_SIZE = 2147483648;// 67108864; // 1024/16
    static const uint32_t HOTKEY_MAX_INDEX_SIZE = 630433;// 1024/4 // 55,924,072
    
    // std::atomic<uint64_t> set_counting;
    // spinlock spl;
    hot_key_index key_index_offsets[HOTKEY_MAX_INDEX_SIZE];
    // char address[ADDRESS_SIZE];
public:

    static uint32_t hothash(char *str)
    {
        uint32_t hash = *((uint32_t*)str);
        // uint32_t hash = XXH64(str, 16, 0);

        // uint32_t seed = 131; // 31 131 1313 13131 131313 etc..
        // uint32_t hash = 0;
        // for(int i=0; i<16;++i){
        //     hash = hash * seed + (*str++);
        // }
        
        return hash % HOTKEY_MAX_INDEX_SIZE;
    }

    /** Initialize your data structure here. */
    HotKeyPool()
    {
        memset(key_index_offsets, 0x00, sizeof(key_index_offsets));
    }

    ~HotKeyPool() {}

    /** value will always be non-negative. */

    void setKV(const Slice &key, char* val, uint16_t valuesize, uint32_t key_pos_, bool is_updated = false){

        if(!is_updated){
            memcpy(key_index_offsets[key_pos_].key, key.data(), Config::KEY_SIZE);
        }

        key_index_offsets[key_pos_].value_size = valuesize;

        memcpy(key_index_offsets[key_pos_].value, val, valuesize);
    }

    bool getKV(const Slice &key, std::string *value, uint32_t *key_pos_){
        // uint32_t key_pos;
        bool success = get_k(key.data(), key_pos_);

        if(success){
            *value = std::string(key_index_offsets[*key_pos_].value, key_index_offsets[*key_pos_].value_size);
            return true;
        }else{
            return false;
        }
    }

    bool get_k(char *key, uint32_t *key_pos){
        *key_pos = hothash(key);

        if(key_index_offsets[*key_pos].value_size == 0){
            return false;
        }else{
            int ret = -1;
            ret = memcmp(key, key_index_offsets[*key_pos].key, Config::KEY_SIZE); // 命中
            if(ret == 0){
                return true;
            }
            return false; // 冲突
        }
    }

    // void setKV(const Slice &key, char* val, uint16_t valuesize, uint32_t key_pos_, bool is_updated = false){

    //     if(!is_updated){
    //         memcpy(key_index_offsets[key_pos_].key, key.data(), Config::KEY_SIZE);
    //     }

    //     key_index_offsets[key_pos_].value_size = valuesize;

    //     memcpy(key_index_offsets[key_pos_].value, val, valuesize);
    // }

    // bool getKV(const Slice &key, std::string *value, uint32_t *key_pos_){
    //     // uint32_t key_pos;
    //     bool success = get_k(key.data(), key_pos_);

    //     if(success){
    //         *value = std::string(key_index_offsets[*key_pos_].value, key_index_offsets[*key_pos_].value_size);
    //         return true;
    //     }else{
    //         return false;
    //     }
    // }

    // bool get_k(char *key, uint32_t *key_pos){

    //     uint32_t k = hothash(key);

    //     while(key_index_offsets[k].value_size != 0){
    //         int ret = -1;
    //         ret = memcmp(key, key_index_offsets[*key_pos].key, Config::KEY_SIZE); // 命中
    //         if(ret == 0){
    //             *key_pos = k;
    //             return true;
    //         }

    //         if( ++k == HOTKEY_MAX_INDEX_SIZE){
    //             k = 0;
    //         }
    //     }

    //     *key_pos = k;
    //     return false;
    // }

};

#endif // TAIR_CONTEST_KV_CONTEST_HOT_H_
