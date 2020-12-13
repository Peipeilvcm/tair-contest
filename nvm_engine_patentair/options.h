// Copyright (c) 2018 The atendb Authors. All rights reserved.

#ifndef TAIR_CONTEST_KV_CONTEST_OPTIONS_H_
#define TAIR_CONTEST_KV_CONTEST_OPTIONS_H_

#include "comparator.h"
#include "env.h"

class Options {
 public:
  Options() :
    comparator_(nullptr) { }

  ~Options() {}

  Comparator* comparator_;
};


#endif // TAIR_CONTEST_KV_CONTEST_OPTIONS_H_
