#ifndef TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_
#define TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_

#include <string>
#include <atomic>
#include "spinlock.h"

class AepFile {
 public:
  AepFile() { }
  virtual ~AepFile() {}

  virtual bool Read(uint64_t offset, uint32_t n, char* buf) = 0;

  virtual bool AppendData(const std::string& key,
			  const std::string& value,
			  uint64_t* file_offset) = 0;

  virtual bool AppendIndex(const std::string& key,
			   uint32_t file_index,
			   uint64_t file_offset,
			   uint32_t key_size_,
			   uint32_t value_size_) = 0;

  virtual bool Close() = 0; 

  virtual uint64_t FileOffset() = 0;
};

class PosixAepFile : public AepFile{
public:
    PosixAepFile(char* fileptr, char* fileendptr, const std::string& fname, size_t file_offset) :
    fileptr_(fileptr),
    fileendptr_(fileendptr),
    fname_(fname),
    file_offset_(file_offset),
    buf_(nullptr),
    base_buf_(nullptr),
    map_size_(4096 * 10),
    map_pos_(0),
    map_avail_(0) { }

    virtual ~PosixAepFile();

    bool Read(uint64_t offset, uint32_t n, char* buf); 

    bool AppendData(const std::string& key, 
            const std::string& value,
            uint64_t* file_offset);

    bool AppendIndex(const std::string& key,
            uint32_t file_index,
            uint64_t file_offset,
            uint32_t key_size_,
            uint32_t value_size_);
    
    bool Close();

    inline uint64_t FileOffset() { return file_offset_; }

private:
    char* fileptr_;
    char* fileendptr_;
    std::string fname_;
    std::atomic<uint64_t> file_offset_;

    char* buf_;
    char* base_buf_;
    uint32_t map_size_;
    uint32_t map_pos_;
    uint32_t map_avail_;

    SpinLock file_lock_;
};

#endif // TAIR_CONTEST_KV_CONTEST_POSIX_AEPFILE_H_