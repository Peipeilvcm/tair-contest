// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_ENV_H_
#define TAIR_CONTEST_KV_CONTEST_ENV_H_

#include <string>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "posix_file.h"
#include "posix_file_aep.h"

#include <libpmem.h>

class Env {
 public:
  Env() {}
  virtual ~Env() {}

  static bool NewFile(const std::string& fname, AepFile** file, uint32_t file_count, bool is_index = false) {
    size_t mapped_len;
    int is_pmem;

    // uint64_t SINGLE_PMEM_LEN = is_index ? 6442450944 / file_count: 68719476736 / file_count;

    uint64_t SINGLE_PMEM_LEN = is_index ? 6442450944 / file_count: 68719476736 / file_count;

    /* create a pmem file and memory map it     printf("%s, offset=%lld\n", fname.c_str(), fileendoffset);*/
    char *flieptr = nullptr;
    if ((flieptr = (char *)pmem_map_file(fname.c_str(), SINGLE_PMEM_LEN, PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem)) == NULL) {
      perror("pmem_map_file");
      exit(1);
    }

    char *fileendianptr;
    std::string fileendian = fname + std::string("-endianptr");
    uint64_t fileendoffset = 0;

    if ((fileendianptr = (char *)pmem_map_file(fileendian.c_str(), sizeof(uint64_t), PMEM_FILE_CREATE,0666, &mapped_len, &is_pmem)) == NULL) {
      perror("pmem_map_file");
      exit(1);
    }

    if (FileExists(fileendian)){
      memcpy(&fileendoffset, fileendianptr, sizeof(uint64_t));
    } 

    *file = new PosixAepFile(flieptr, fileendianptr, fname, fileendoffset);

    return true;
  }

  static bool FileExists(const std::string& fname) {
    if (access(fname.c_str(), F_OK) == 0) {
      return true;
    } else{
      return false;
    }
  }

  static bool CreateDir(const std::string& dirname) {
    if (mkdir(dirname.c_str(), 0755) != 0) {
      return false;
    }
    return true;
  }

  static bool GetFileSize(const std::string& fname, uint64_t* size) {
    struct stat sbuf;
    if (stat(fname.c_str(), &sbuf) != 0) {
      *size = 0;
      return false;
    } else {
      *size = sbuf.st_size;
    }
    return true;
  }

  static bool GetCurrentDir(std::string* dir) {
    char cwd[125];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
      *dir = std::string(cwd);
      return true;
    }
    return false;
  }

};


#endif //TAIR_CONTEST_KV_CONTEST_ENV_H_
