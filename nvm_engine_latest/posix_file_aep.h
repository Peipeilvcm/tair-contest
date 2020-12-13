#ifndef TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_
#define TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_

#include <string>
#include <atomic>
#include "spinlock.h"
#include "include/db.hpp"
#include "config.h"

#include <vector>
#include <queue>
#include <stack>

// typedef std::priority_queue<std::pair<uint32_t, uint32_t>, std::vector<std::pair<uint32_t, uint32_t> >, std::greater<std::pair<uint32_t, uint32_t> > > free_space_entry;
// typedef std::queue<uint32_t> free_space_entry;
typedef std::stack<uint32_t> free_space_entry;



class PosixAepFile
{
public:
    PosixAepFile(char *fileptr, const std::string &fname, uint32_t file_offset) : fileptr_(fileptr),
                                                                                                    fname_(fname),
                                                                                                    file_end_offset_(file_offset) {
                                                                                                    }

    PosixAepFile(char *fileptr, const std::string &fname) : fileptr_(fileptr),
                                                            fname_(fname)
                                                             {
        int iterval = 1<<Config::CHUNK_SIZE;
        int min_item_size  = ((107 + (iterval - 1)) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
        int max_item_size  = ((1051 + (iterval - 1)) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
        int cnts = ((max_item_size - min_item_size) / iterval) + 1;

        for(int i = 0; i < cnts; i++){
            free_space_entry* fs = new free_space_entry();
            free_space.push_back(fs);
        }
        this->key_set_cnts_f = 0;
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
    void ReadKV_KVs(uint32_t offset, char* key, uint16_t *value_size_, uint32_t *key_set_cnts, uint8_t *chunk_id);
    void ReadKV_SetCnts(uint32_t offset, uint32_t *key_set_cnts);
    void AppendKV_Data(const Slice &key, const Slice &value, uint32_t* index_head_offset);
    // void InsertKV_Data(const Slice &key, const Slice &value, uint32_t *value_head_offset);
    void UpdateKV_Data(const Slice &key, const Slice &value, uint32_t *value_head_offset, const uint32_t& old_index_offset, const uint32_t& old_key_set_cnts);
    void SetKV_Data(uint32_t offset, const Slice &key, const Slice &value, const uint32_t& key_set_cnt);
    bool CMP_KEY(uint32_t offset, char *key);
    void RecycleKV_Data(const uint32_t& old_index_offset);
    // void addFreeSpace(uint32_t old_index_offset);
    void CreateKV_Data(uint32_t offset, const Slice &key, const Slice &value, const uint32_t& key_set_cnt);

    static inline int calFreeSpaceIdx(uint16_t value_size, uint16_t* item_size_){
        uint16_t entry_truth_size = Config::KEY_SIZE + 11 + value_size;
        int item_size  = ((entry_truth_size + (1<<Config::CHUNK_SIZE) - 1) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;

        if(item_size_ != nullptr){
            *item_size_ = item_size;
        }

        int min_item_size  = ((107 + (1<<Config::CHUNK_SIZE) - 1) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
        int idx = (item_size - min_item_size) / (1<<Config::CHUNK_SIZE);
        return idx;
    }
    static inline uint16_t calSizeFromIdx(uint8_t idx){
        int iterval = 1<<Config::CHUNK_SIZE;
        int min_item_size  = ((107 + (iterval - 1)) >> Config::CHUNK_SIZE) << Config::CHUNK_SIZE;
        return min_item_size + idx*iterval;
    }

    void setKeySetCnts(uint32_t key_set_cnts){
        key_set_cnts_f = key_set_cnts;
    }

    // void findUsefulSpace(uint8_t start_chunk_id, uint32_t *index_head_offset);


private:
    char *fileptr_;
    
    std::string fname_;
    uint32_t file_end_offset_;
    uint32_t key_set_cnts_f;

    // key_map hot_key_array[Config::KEY_MAP_SIZE]; //key map

    // std::vector<std::queue<uint32_t>* > free_space;
    std::vector<free_space_entry* > free_space;
    // std::vector<spinlock* > free_space_spls;
    spinlock free_space_spls[64];
    // std::vector<free_space_entry* > free_space;

    // char *buf_;

    spinlock file_end_offset_spl, key_set_cnts_f_spl;
    spinlock spl;
    // SpinLock file_lock_;
    // std::mutex mutex;
};

#endif // TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_