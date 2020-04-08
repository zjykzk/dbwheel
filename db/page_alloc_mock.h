// Copyright (c) 2020
//
#ifndef DB_PAGE_ALLOC_MOCK_H_
#define DB_PAGE_ALLOC_MOCK_H_

#include <map>
#include <vector>
#include <memory>

#include "db/page.h"
#include "db/page_alloc.h"

namespace dbwheel {

using std::shared_ptr;
using std::vector;

struct MockPageAlloc: public PageAlloc {
  MockPageAlloc(uint64_t nextPageID): nextPageID(nextPageID) {}

  Page* alloc(size_t sz, size_t count) {

    char* buf = new char[count * sz];
    bufs.push_back(shared_ptr<char>(buf, [](char* b) {delete[] b;}));

    Page* p = new (buf) Page(nextPageID++, static_cast<uint32_t>(count - 1));
    alloced[p->id()] = p;

    return p;
  }

  std::map<uint64_t, Page*> alloced;
  vector<shared_ptr<char> > bufs;

  uint64_t nextPageID;
};

}  // namespace dbwheel

#endif  // DB_PAGE_ALLOC_MOCK_H_



