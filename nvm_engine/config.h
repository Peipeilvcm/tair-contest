#ifndef TAIR_CONTEST_KV_CONTEST_CONFIG_H_
#define TAIR_CONTEST_KV_CONTEST_CONFIG_H_

#include<stdint.h>

typedef uint32_t index_offset_t;
typedef uint32_t data_offset_t;

class Config
{
public:

    static const uint8_t MAX_FILE_COUNT = 16;

    // static const uint32_t MAX_INDEX_SIZE_MOD = 1073741823;
    // static const uint32_t MAX_INDEX_SIZE = 1073741824; // 1024*1024*1024

    static const uint32_t MAX_INDEX_SIZE_SHARD_MOD = 67108863; //
    static const uint32_t MAX_INDEX_SIZE_SHARD = 67108864; // 1024/16
    // static const uint32_t MAX_INDEX_SIZE_MOD = 2147483647;
    // static const uint32_t MAX_INDEX_SIZE = 1879048192; // 2*1024*1024*1024 - 256*1024*1024
    static const uint32_t INDEX_OFFSET_EMPTY = 0xFFFFFFFF;
    static const uint32_t DATA_OFFSET_EMPTY = 0xFFFFFFFF;
    static const uint64_t SINGLE_PMEM_LEN = 1073741824; // 1024*1024*1024

    // static const uint64_t DATA_PMEM_MAX_SIZE = 62277025792; // 62277025792; // 58G
    // static const uint64_t INDEX_PMEM_MAX_SIZE = 6442450944; // 6G

    static const uint64_t TOTAL_PMEM_MAX_SIZE =68719476736; // 64G

    static const uint8_t KEY_SIZE = 16;
    // static const uint8_t DATA_BASE_SIZE = 128;
    // static const uint8_t FIRST_DATA_BASE_SIZE = 126;


    // static const uint32_t INIT_OFFSET_128 = 0;//[1+164M+14, 1+54M-2, 0+485-2, 0+646-2, 0+135-2, 0+161-2, 188-2, 215-2]
    // static const uint32_t INIT_OFFSET_256 = 1260388352; //1024+178
    // static const uint32_t INIT_OFFSET_384 = 2388656128;//1024+52
    // static const uint32_t INIT_OFFSET_512 = 2895118336;//0+483
    // static const uint32_t INIT_OFFSET_640 = 3570401280;//644
    // static const uint32_t INIT_OFFSET_768 = 3709861888;//133
    // static const uint32_t INIT_OFFSET_896 = 3876585472;//159
    // static const uint32_t INIT_OFFSET_1024 = 4071620608; //186

    // static const uint32_t INIT_OFFSET_BY_VALUESIZE_ARRAY[8];

    static const uint8_t CHUNK_SIZE = 4; //4bits 2^4 16 

    // char *fileptr_128;//57.2
    // char *fileptr_256;//48
    // char *fileptr_384;
    // char *fileptr_512;
    // char *fileptr_640;
    // char *fileptr_768;
    // char *fileptr_896;
    // char *fileptr_1024;

    static FILE* global_log_ptr;

    Config(){}
    ~Config(){}
};


// struct index_entry{
//     char key_[Config::KEY_SIZE];
//     uint32_t value_head_offset;
//     // uint16_t value_size;
// };

// struct data_entry{
//     char data_[Config::DATA_BASE_SIZE]; //key + [value_size] + next_file_offset
//     uint32_t next_file_offset;
// };

// struct first_data_entry{
//     uint16_t value_size;
//     char data_[Config::FIRST_DATA_BASE_SIZE]; //key + [value_size] + next_file_offset
//     uint32_t next_file_offset;
// };

// struct kv_entry{
//     char key_[Config::KEY_SIZE];
//     uint16_t value_size;
//     char* value;
//     // char data_[Config::FIRST_DATA_BASE_SIZE]; //key + [value_size] + next_file_offset
//     // uint32_t next_file_offset;
// };


#endif //TAIR_CONTEST_KV_CONTEST_CONFIG_H_