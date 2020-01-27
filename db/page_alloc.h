// Copyright (c) 2020
//
#ifndef DB_PAGE_ALLOC_H_
#define DB_PAGE_ALLOC_H_

namespace dbwheel {

class Page;

struct PageAlloc {
  virtual Page* alloc(size_t sz, size_t count) = 0;
};

}  // namespace dbwheel

#endif  // DB_PAGE_ALLOC_H_


