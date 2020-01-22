// Copyright (c) 2020
//
#ifndef DB_PAGE_H_
#define DB_PAGE_H_

#include <cstdint>

#include <string>

namespace dbwheel {

using pgid = uint64_t;

struct branchPageElement;

struct leafPageElement;

class Page {
 private:
  const std::string type();

  const uint32_t count() { return count_; }

  branchPageElement* branchPageElements() {
    return reinterpret_cast<branchPageElement*>(this->ptr_);
  }

  inline branchPageElement* branchPageElementOf(uint16_t index);

  leafPageElement* leafPageElements() {
    return reinterpret_cast<leafPageElement*>(this->ptr_);
  }

  inline leafPageElement* leafPageElementOf(uint16_t index);

  pgid id_;
  uint16_t flags_;
  uint16_t count_;
  uint32_t overflow_;
  char ptr_[0];
};

}  // namespace dbwheel

#endif  // DB_PAGE_H_
