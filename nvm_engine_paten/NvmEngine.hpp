#ifndef TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
#define TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_

#include <atomic>
#include <algorithm>

#include "include/db.hpp"
#include "posix_file.h"
#include "options.h"
#include <vector>


#include "skiplist.h"
const std::string IndexFileName = "/INDEX-";
const std::string DataFileName = "/DATA-";

typedef SkipList<Index, Compare> Table;

class NvmEngine : DB {
public:
    /**
     * @param 
     * name: file in AEP(exist)
     * dbptr: pointer of db object
     *
     */
    static Status CreateOrOpen(const std::string &name, DB **dbptr);
    NvmEngine(const std::string &name);
    Status Get(const Slice &key, std::string *value);
    Status Set(const Slice &key, const Slice &value);
    ~NvmEngine();

    void IndexCallback(File* file);
    bool Recover();

    // NvmEngine(const std::string &name);

private:
    std::string dbname_;
    Options options_;
    std::atomic<uint32_t> cursor_;

    uint32_t max_file_;
    std::vector<File* > data_files_;
    std::vector<File* > index_files_;

    uint32_t table_size_;
    std::vector<Table *> tables_;

    std::atomic<uint64_t> set_counting;
    std::atomic<uint64_t> get_counting;
};



#endif // TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
