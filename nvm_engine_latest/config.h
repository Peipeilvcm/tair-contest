#ifndef TAIR_CONTEST_KV_CONTEST_CONFIG_H_
#define TAIR_CONTEST_KV_CONTEST_CONFIG_H_

#include<stdint.h>

typedef uint32_t index_offset_t;
typedef uint32_t data_offset_t;

class Config
{
public:

    static const uint8_t MAX_FILE_COUNT = 16;
    static const uint8_t MAX_FILE_COUNT_MOD = 15;
    static const uint32_t MAX_INDEX_SIZE_SHARD = 33554467; //512M 102274681;//92274671 67108859 67108864; // 1024/16
    static const uint32_t MAX_FREE_SPACE_SIZE = 1000000;
    // static const uint8_t MAX_FILE_COUNT = 32;
    // static const uint8_t MAX_FILE_COUNT_MOD = 31;
    // static const uint32_t MAX_INDEX_SIZE_SHARD = 33554393;// 33554432; // 1024/16
    // static const uint32_t MAX_FREE_SPACE_SIZE = 1223334;
    
    static const uint32_t INDEX_OFFSET_EMPTY = 0xFFFFFFFF;

    static const uint64_t TOTAL_PMEM_MAX_SIZE =68719476736; // 64G

    static const uint64_t KEY_OFFSET_SIZE = 16777216; //268435456;//256M

    static const uint8_t KEY_SIZE = 16;

    static const uint32_t KEY_MAP_SIZE = 128813;

    static const uint8_t CHUNK_SIZE = 5; //4bits 2^4 16 

    static FILE* global_log_ptr;

    Config(){}
    ~Config(){}
};



#endif //TAIR_CONTEST_KV_CONTEST_CONFIG_H_