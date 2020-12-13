#ifndef TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_
#define TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_

#include <string>
#include <atomic>
#include "spinlock.h"
#include "include/db.hpp"
#include "config.h"

#include <vector>
#include <queue>

class PosixAepFile
{
public:
    PosixAepFile(char *fileptr, const std::string &fname, uint32_t file_offset) : fileptr_(fileptr),
                                                                                                    fname_(fname),
                                                                                                    file_end_offset_(file_offset),
                                                                                                    buf_(nullptr) {
                                                                                                    }

    PosixAepFile(char *fileptr, const std::string &fname) : fileptr_(fileptr),
                                                            fname_(fname),
                                                            buf_(nullptr) {
    int iterval = 1<<Config::CHUNK_SIZE;
    int min_item_size  = ((80 + 20 + (iterval - 1)) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
    int max_item_size  = ((1024 + 20 + (iterval - 1)) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
    for(int i = min_item_size; i <= max_item_size; i += iterval){
        std::queue<uint32_t>* fs = new std::queue<uint32_t>();
        free_space.push_back(fs);
    }
                                                            }

    virtual ~PosixAepFile();


    void Close();

    inline uint32_t FileEndOffset() { return file_end_offset_; }

    // void setFileEndPtr(char* fileendptr){
    //     fileendptr_ = fileendptr;
    // }
    void setFileEndOffset(uint32_t file_end_offset){
        // fileendptr_ = fileendptr;
        file_end_offset_ = file_end_offset;
    }

    void ReadDataEntry(uint32_t offset, char *buf);

    void ReadKV_Data(uint32_t offset, uint16_t *value_size_, char* value);
    void ReadKV_KVs(uint32_t offset, char* key, uint16_t *value_size_, uint8_t *key_set_cnts, uint8_t *chunk_id);
    void ReadKV_SetCnts(uint32_t offset, uint8_t *key_set_cnts);
    void InsertKV_Data(const Slice &key, const Slice &value, uint8_t key_set_cnt, uint32_t *value_head_offset);
    void UpdateKV_Data(const Slice &key, const Slice &value, uint32_t *value_head_offset, uint32_t old_index_offset, uint8_t old_key_set_cnt);
    void SetKV_Data(uint32_t offset, const Slice &key, const Slice &value, uint8_t key_set_cnt);
    bool CMP_KEY(uint32_t offset, char *key);
    void RecycleKV_Data(uint32_t old_index_offset);
    void CreateKV_Data(uint32_t offset, const Slice &key, const Slice &value, uint8_t key_set_cnt);

    static inline int calFreeSpaceIdx(uint16_t value_size, uint16_t* item_size_){
        uint16_t entry_truth_size = Config::KEY_SIZE + 4 + value_size;
        int item_size  = ((entry_truth_size + (1<<Config::CHUNK_SIZE) - 1) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;

        if(item_size_ != nullptr){
            *item_size_ = item_size;
        }

        int min_item_size  = ((100 + (1<<Config::CHUNK_SIZE) - 1) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
        int idx = (item_size - min_item_size) / (1<<Config::CHUNK_SIZE);
        return idx;
    }
    static inline uint16_t calSizeFromIdx(uint8_t idx){
        int iterval = 1<<Config::CHUNK_SIZE;
        int min_item_size  = ((80 + 20 + (iterval - 1)) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
        return min_item_size + idx*iterval;
    }


private:
    char *fileptr_;
    
    std::string fname_;
    std::atomic<uint32_t> file_end_offset_;

    std::vector<std::queue<uint32_t>* > free_space;

    char *buf_;

    spinlock spl;
    // SpinLock file_lock_;
    // std::mutex mutex;
};

#endif // TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_