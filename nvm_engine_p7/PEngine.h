#ifndef ENGINE_RACE_PENGINE_H
#define ENGINE_RACE_PENGINE_H

#include <malloc.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <aio.h>

#include <unistd.h>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include <map>
#include <set>
// #include <condition_variable>

#include "SortLog.h"
#include "KVFiles.h"
#include "KeyValueLog.h"


#include "include/db.hpp"

using namespace std;
using namespace std::chrono;



//    milliseconds now() {
//        return duration_cast<milliseconds>(system_clock::now().time_since_epoch());
//    }


class PEngine {

    private:
        milliseconds start;
        u_int32_t set_count;
        u_int32_t get_count;
    

public:
    explicit PEngine(const string &path);

    ~PEngine();

    void Set(const Slice &key, const Slice &value);

    Status Get(const Slice &key, string *value);


    // void readDisk();

    // void rangeAll(Visitor &visitor);

    static inline int getLogId(const char *k) {
        return ((u_int16_t) ((u_int8_t) k[0]) << 4) | ((u_int8_t) k[1] >> 4);
    }

};


#endif //ENGINE_RACE_PENGINE_H