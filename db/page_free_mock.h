// Copyright (c) 2020
//
#ifndef DB_PAGE_FREE_MOKC_H_
#define DB_PAGE_FREE_MOKC_H_

#include "db/page_free.h"
#include <vector>

namespace dbwheel {

struct MockPageFree: public PageFree {
  void free(uint64_t pageID) override { freed.push_back(pageID); }

  std::vector<uint64_t> freed;
};

}  // namespace dbwheel

#endif   // DB_PAGE_FREE_MOKC_H_

