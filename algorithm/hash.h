// Copyright (c) 2014, Emmanuel Goossaert. All rights reserved.
// Use of this source code is governed by the BSD 3-Clause License,
// that can be found in the LICENSE file.

#ifndef NVMDB_HASH_H_
#define NVMDB_HASH_H_

// #include "util/debug.h"
#include <string>
#include <cstdint>
#include <limits>
#include <string.h>

// #include "util/logger.h"
// #include "util/options.h"
#include "murmurhash3.h"
#include "xxhash.h"

namespace nvmdb
{

  enum HashType
  {
    kMurmurHash3_64 = 0x0,
    kxxHash_64 = 0x1
  };

  class Hash
  {
  public:
    Hash() {}
    virtual ~Hash() {}
    virtual uint64_t HashFunction(const char *data, uint32_t len) = 0;
    virtual uint64_t MaxInputSize() = 0;
  };

  class MurmurHash3 : public Hash
  {
  public:
    MurmurHash3() {}
    virtual ~MurmurHash3() {}
    virtual uint64_t HashFunction(const char *data, uint32_t len);
    virtual uint64_t MaxInputSize() { return std::numeric_limits<int32_t>::max(); }
  };

  class xxHash : public Hash
  {
  public:
    xxHash() {}
    virtual ~xxHash() {}
    virtual uint64_t HashFunction(const char *data, uint32_t len);
    virtual uint64_t MaxInputSize() { return std::numeric_limits<int32_t>::max(); }
  };

  Hash *MakeHash(HashType ht);

} // namespace nvmdb

#endif // NVMDB_HASH_H_
