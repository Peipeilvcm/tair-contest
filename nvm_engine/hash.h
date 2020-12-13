// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_HASH_H_
#define TAIR_CONTEST_KV_CONTEST_HASH_H_

#include <stdint.h>
#include <string.h>
#include "config.h"

#include "posix_file_aep.h"
#include "spinlock.h"


uint32_t hash(char *str)
{
    uint32_t hash = (*(uint32_t*)str);
    
    return hash & Config::MAX_INDEX_SIZE_SHARD_MOD;
}

struct index_offset_entry
{
    uint16_t file_offset_l;
    uint8_t file_offset_r;
};

class MyHashMap
{
private:
    spinlock spl;
    uint32_t index_offsets[Config::MAX_INDEX_SIZE_SHARD];
    // std::mutex mutex;
public:

    /** Initialize your data structure here. */
    MyHashMap()
    {
        memset(index_offsets, 0xFF, sizeof(index_offsets));
        // memset(flags, false, sizeof(flags));
    }

    ~MyHashMap() {}

    /** value will always be non-negative. */
    void put(char *key, uint32_t offset)
    {

        uint32_t k = hash(key);

        spl.lock();
        while (~index_offsets[k]) // 空的话停止
        {
            if (++k == Config::MAX_INDEX_SIZE_SHARD)
            {
                k = 0;
            }
        }
        // lock_.Lock();
        
        // flags[k] = true;
        // index_offsets[k] = uint32_t_to_index_entry(offset);
        index_offsets[k] = offset;
        spl.unlock();
        // lock_.Unlock();
    }

    void update(uint32_t hash_pos, uint32_t offset){
        spl.lock();
        index_offsets[hash_pos] = offset;
        spl.unlock();
    }

    uint32_t get(char *key, PosixAepFile* index_file, uint32_t *hash_pos, uint8_t* key_set_cnt)
    {
        uint32_t k = hash(key);

        spl.lock();
        while (~index_offsets[k]) 
        {
            if (index_file->CMP_KEY(index_offsets[k], key))
            {
                *hash_pos = k;
                index_file->ReadKV_SetCnts(index_offsets[k], key_set_cnt);
                spl.unlock();
                return index_offsets[k];
            }

            if (++k == Config::MAX_INDEX_SIZE_SHARD)
            {
                k = 0;
            }
        }
        spl.unlock();
        return Config::INDEX_OFFSET_EMPTY;
    }

    uint32_t get(char *key, PosixAepFile* index_file)
    {
        uint32_t k = hash(key);

        spl.lock();
        while (~index_offsets[k]) 
        {
            if (index_file->CMP_KEY(index_offsets[k], key))
            {
                spl.unlock();
                return index_offsets[k];
            }

            if (++k == Config::MAX_INDEX_SIZE_SHARD)
            {
                k = 0;
            }

        }
        spl.unlock();
        return Config::INDEX_OFFSET_EMPTY;
    }

    uint32_t index_entry_to_uint32_t(const index_offset_entry &iden)
    {
        uint32_t offset = iden.file_offset_l << 8 | iden.file_offset_r;
        return offset;
    }

    index_offset_entry uint32_t_to_index_entry(const uint32_t &offset)
    {
        index_offset_entry iden;
        iden.file_offset_l = (offset & 0x00FFFF00) >> 8;
        iden.file_offset_r = (offset & 0x000000FF);
        return iden;
    }

};

#endif // TAIR_CONTEST_KV_CONTEST_HASH_H_
