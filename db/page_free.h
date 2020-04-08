// Copyright (c) 2020
//
#ifndef DB_PAGE_FREE_H_
#define DB_PAGE_FREE_H_

#include <cstdint>

namespace dbwheel {

struct PageFree {
  virtual void free(uint64_t pageID) = 0;
};

}  // namespace dbwheel

#endif  // DB_PAGE_FREE_H_

