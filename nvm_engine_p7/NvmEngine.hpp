#ifndef TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_
#define TAIR_CONTEST_KV_CONTEST_NVM_ENGINE_H_


#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <mutex>

#include "include/db.hpp"

#include <unordered_map>
#include <map>
#include <sys/time.h>
#include <thread>

#include "PEngine.h"



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

private:
    PEngine *pEngine;
};



#endif
