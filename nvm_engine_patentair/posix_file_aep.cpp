#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <cstring>

#include "posix_file_aep.h"

#include "coding.h"

#include <libpmem.h>

PosixAepFile::~PosixAepFile() {
    this->Close();
}

bool PosixAepFile::Read(uint64_t offset, uint32_t n, char* buf) {

    if (offset > file_offset_) {
        uint32_t pos = offset - file_offset_;
        if (pos > map_size_) {
            return false;
        } else {
            memcpy(buf, base_buf_+pos, n);
        }
    } else {
        memcpy(buf, fileptr_+offset, n);
    }
    return true;
}

bool PosixAepFile::AppendData(const std::string& key,
			   const std::string& value,
			   uint64_t* file_offset) {
    uint32_t data_size = key.size() + value.size();
    char data_buf[data_size];
    EncodeData(data_buf, key, value);

    *file_offset = file_offset_.fetch_add(data_size);

    memcpy(fileptr_ + *file_offset, data_buf, data_size);
    pmem_persist(fileptr_ + *file_offset, data_size);

    // 持久化存储 文件末尾
    uint64_t fileend = file_offset_;
    memcpy(fileendptr_, &fileend, sizeof(uint64_t));
    pmem_persist(fileendptr_, sizeof(uint64_t));

    return true;
}

bool PosixAepFile::AppendIndex(const std::string& key,
			    uint32_t file_index,
			    uint64_t file_offset,
			    uint32_t key_size,
			    uint32_t value_size) {
    uint32_t index_size = key.size() + sizeof(uint32_t) * 4 + sizeof(uint64_t);
    char index_buf[index_size];
    EncodeIndex(index_buf, key, file_index,
	        file_offset, key_size, value_size);

    uint64_t offset = file_offset_.fetch_add(index_size);

    memcpy(fileptr_ +  offset, index_buf, index_size);
    pmem_persist(fileptr_ + offset, index_size);

    // 持久化存储 文件末尾
    uint64_t fileend = file_offset_;
    memcpy(fileendptr_, &fileend, sizeof(uint64_t));
    pmem_persist(fileendptr_, sizeof(uint64_t));

    return true;
}

bool PosixAepFile::Close() {
  
  return true;
}
