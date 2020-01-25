// Copyright (c) 2020
//

#ifndef DB_PAGE_ELE_H
#define DB_PAGE_ELE_H

#include <cstdint>

#include <string>

namespace dbwheel {


struct branchPageElement {
  std::string key();

  uint32_t pos;
  uint32_t ksize;
  uint64_t pageID;
};


struct leafPageElement {
  std::string key();
  std::string value();

  uint32_t flags;
  uint32_t pos;
  uint32_t ksize;
  uint32_t vsize;
};

}  // namespace dbwheel

#endif  // DB_PAGE_ELE_H
