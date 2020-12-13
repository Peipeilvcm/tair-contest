// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_ENV_H_
#define TAIR_CONTEST_KV_CONTEST_ENV_H_

#include <string>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "posix_file_aep.h"
#include "config.h"

#include <libpmem.h>

class Env
{
public:
    Env() {}
    virtual ~Env() {}

    static void NewKVFile(const std::string &fname, char *flieptr, uint8_t fd, PosixAepFile **file)
    {
        
        uint64_t SINGLE_PMEM_LEN = Config::TOTAL_PMEM_MAX_SIZE / Config::MAX_FILE_COUNT;

        uint64_t head_offset = SINGLE_PMEM_LEN * fd;
        flieptr = flieptr + head_offset;
        *file = new PosixAepFile(flieptr, fname);
    }


    static bool NewFile(const std::string &fname, char *flieptr, uint8_t fd, PosixAepFile **file, bool is_index = false)
    {
        // uint64_t SINGLE_INDEX_PMEM_LEN = Config::TOTAL_PMEM_MAX_SIZE / Config::MAX_FILE_COUNT;
        // uint64_t SINGLE_DATA_PMEM_LEN = Config::DATA_PMEM_MAX_SIZE / Config::MAX_FILE_COUNT;

        // if(is_index){
        //     uint64_t head_offset = SINGLE_INDEX_PMEM_LEN * fd;
        //     flieptr = flieptr + head_offset;
        //     *file = new PosixAepFile(flieptr, fname);
        // }else{
        //     uint64_t head_offset = SINGLE_DATA_PMEM_LEN * fd +  Config::INDEX_PMEM_MAX_SIZE;
        //     flieptr = flieptr + head_offset;
        //     *file = new PosixAepFile(flieptr, fname);
        // }
        
        // // fprintf(log_file, "New File successfully %s\n",fname.c_str());

        return true;
    }

    static bool FileExists(const std::string &fname)
    {
        if (access(fname.c_str(), F_OK) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // static bool CreateDir(const std::string &dirname)
    // {
    //     if (mkdir(dirname.c_str(), 0755) != 0)
    //     {
    //         return false;
    //     }
    //     return true;
    // }

    // static bool GetFileSize(const std::string &fname, uint64_t *size)
    // {
    //     struct stat sbuf;
    //     if (stat(fname.c_str(), &sbuf) != 0)
    //     {
    //         *size = 0;
    //         return false;
    //     }
    //     else
    //     {
    //         *size = sbuf.st_size;
    //     }
    //     return true;
    // }

    // static bool GetCurrentDir(std::string *dir)
    // {
    //     char cwd[125];
    //     if (getcwd(cwd, sizeof(cwd)) != nullptr)
    //     {
    //         *dir = std::string(cwd);
    //         return true;
    //     }
    //     return false;
    // }
};

#endif //TAIR_CONTEST_KV_CONTEST_ENV_H_
