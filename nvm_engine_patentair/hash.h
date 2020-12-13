// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_HASH_H_
#define TAIR_CONTEST_KV_CONTEST_HASH_H_

unsigned int Hash(const char* str) {
  unsigned int seed = 131;
  unsigned int hash = 0;

  while (*str) {
    hash = hash * seed + (*str++);
  }
  return (hash & 0x7FFFFFFF);
}

#endif // TAIR_CONTEST_KV_CONTEST_HASH_H_
