// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_INDEX_H_
#define TAIR_CONTEST_KV_CONTEST_INDEX_H_

// #include <string>
// #include <string.h>

// #define KEY_SIZE 16
// #define DATA_BASE_SIZE 128




// class Index {
//   public:
//     Index() {}

//     Index(const char* &key) :
//       file_offset_(0),
//       value_size_(0) {
//         memcpy(this->key_, key, KEY_SIZE);
//       }

//     Index(const char* &key, uint64_t file_offset, uint16_t value_size) :
//       file_offset_(file_offset),
//       value_size_(value_size) {
//         memcpy(this->key_, key, KEY_SIZE);
//       }

//     ~Index() {} 

//     void operator=(const Index& index) {
//       memcpy(this->key_, index.key_, KEY_SIZE);
//       this->file_offset_ = index.file_offset_;
//       this->value_size_ = index.value_size_;
//     }

//     char key_[KEY_SIZE];
//     uint64_t file_offset_;
//     uint16_t value_size_;

//   private:
//     friend class NvmEngine;
// };

#endif // TAIR_CONTEST_KV_CONTEST_INDEX_H_

