#ifndef TAIR_CONTEST_KV_CONTEST_CODING_H_
#define TAIR_CONTEST_KV_CONTEST_CODING_H_

#include <iostream>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)


void EncodeData(char* buf,
		const std::string& key,
		const std::string& value);

void DecodeData(char* buf,
		uint32_t key_size,
		uint32_t value_size,
		std::string* value);

void EncodeIndex(char* buf,
		 const std::string& key,
		 uint32_t file_index,
		 uint64_t file_offset,
		 uint32_t key_size,
		 uint32_t value_size);

void DecodeIndex(char* buf,
		 std::string* key,
		 uint32_t* file_index,
		 uint64_t* file_offset,
		 uint32_t* key_size,
		 uint32_t* value_size);

uint32_t DecodeFixed32(const char* buf);

uint64_t DecodeFixed64(const char* buf);


#endif //TAIR_CONTEST_KV_CONTEST_CODING_H_
