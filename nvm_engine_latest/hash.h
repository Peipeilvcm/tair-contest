// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_HASH_H_
#define TAIR_CONTEST_KV_CONTEST_HASH_H_

#include <stdint.h>
#include <string.h>
#include "config.h"

#include "posix_file_aep.h"
#include "spinlock.h"
#include "xxhash.h"


uint32_t hash(char *str)
{
    // uint32_t hash = XXH64(str, 16, 0);
    uint32_t hash = *((uint32_t*)str);
    // uint32_t seed = 131; // 31 131 1313 13131 131313 etc..
    // uint32_t hash = 0;
    // for(int i=0; i<16;++i){
    //     hash = hash * seed + (*str++);
    // }
    
    return hash % Config::MAX_INDEX_SIZE_SHARD;
}

// struct index_offset_entry
// {
//     uint16_t file_offset_l;
//     uint8_t file_offset_r;
// };

struct key_offset_entry
{
    char key[Config::KEY_SIZE];
    uint32_t offset;
};



class MyHashMap
{
private:
    spinlock spl;
    uint32_t index_offsets[Config::MAX_INDEX_SIZE_SHARD];
    key_offset_entry key_offset_s[Config::KEY_OFFSET_SIZE];
    std::atomic<uint32_t> key_offset_s_end;
    // std::mutex mutex;
public:

    /** Initialize your data structure here. */
    MyHashMap()
    {
        key_offset_s_end = 0;
        memset(index_offsets, 0xFF, sizeof(index_offsets));
        memset(key_offset_s, 0xFF, sizeof(key_offset_s));
        // memset(flags, false, sizeof(flags));
    }

    ~MyHashMap() {}

    /** value will always be non-negative. */
    void put(char *key, uint32_t offset)
    {

        uint32_t k = hash(key);

        // spl.lock();
        while (~index_offsets[k]) // 空的话停止
        {
            if (++k == Config::MAX_INDEX_SIZE_SHARD)
            {
                k = 0;
            }
        }
        
        // flags[k] = true;
        // index_offsets[k] = uint32_t_to_index_entry(offset);
        // index_offsets[k] = offset;
        index_offsets[k] = key_offset_s_end.fetch_add(1);
        memcpy(key_offset_s[index_offsets[k]].key, key, Config::KEY_SIZE);
        key_offset_s[index_offsets[k]].offset = offset;

        // spl.unlock();
        // lock_.Unlock();
    }

    void update(uint32_t hash_pos, uint32_t offset){
        // spl.lock();

        // index_offsets[hash_pos] = offset;
        key_offset_s[index_offsets[hash_pos]].offset = offset;

        // spl.unlock();
    }

    uint32_t get(char *key, PosixAepFile* index_file, uint32_t *hash_pos, uint32_t* key_set_cnt)
    {
        uint32_t k = hash(key);

        // spl.lock();
        while (~index_offsets[k]) 
        {
            if (memcmp(key, key_offset_s[index_offsets[k]].key, 16) == 0)
            {
                *hash_pos = k;
                index_file->ReadKV_SetCnts( key_offset_s[index_offsets[k]].offset, key_set_cnt);
                // spl.unlock();
                return key_offset_s[index_offsets[k]].offset;
            }

            if (++k == Config::MAX_INDEX_SIZE_SHARD)
            {
                k = 0;
            }
        }
        // spl.unlock();
        return Config::INDEX_OFFSET_EMPTY;
    }

    uint32_t get(char *key, PosixAepFile* index_file)
    {
        uint32_t k = hash(key);

        // spl.lock();
        while (~index_offsets[k]) 
        {
            if (memcmp(key, key_offset_s[index_offsets[k]].key, 16) == 0)
            {
                // spl.unlock();
                return key_offset_s[index_offsets[k]].offset;
            }

            if (++k == Config::MAX_INDEX_SIZE_SHARD)
            {
                k = 0;
            }

        }
        // spl.unlock();
        return Config::INDEX_OFFSET_EMPTY;
    }

};

#endif // TAIR_CONTEST_KV_CONTEST_HASH_H_
