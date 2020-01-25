// Copyright (c) 2020
//
#ifndef DB_PAGE_H_
#define DB_PAGE_H_

#include <cstdint>

#include <string>

namespace dbwheel {

struct branchPageElement;

struct leafPageElement;

class Page {
 private:
  friend class Node;

  const std::string type();

  const uint32_t count() { return count_; }
  void count(uint32_t c) { count_ = c; }
  const uint16_t flags() { return flags_; }
  void flags(uint16_t flags) { flags_ = flags; }
  const uint64_t id() { return id_; }
  void id(uint64_t id) { id_ = id; }

  branchPageElement* branchPageElements() {
    return reinterpret_cast<branchPageElement*>(this->ptr_);
  }

  inline branchPageElement* branchPageElementOf(uint16_t index);

  leafPageElement* leafPageElements() {
    return reinterpret_cast<leafPageElement*>(this->ptr_);
  }

  inline leafPageElement* leafPageElementOf(uint16_t index);

  uint64_t id_;
  uint16_t flags_;
  uint16_t count_;
  uint32_t overflow_;
  char ptr_[0];

  static const size_t kPageHeaderSize;
  static const size_t kBranchPageElementSize;
  static const size_t kLeafPageElementSize;
  static const size_t kMinKeys;

  enum {
    kBranchPageFlag = 0x01,
    kLeafPageFlag = 0x02,
    kMetaPageFlag = 0x04,
    kFreeListPageFlag = 0x10
  };

};

}  // namespace dbwheel

#endif  // DB_PAGE_H_
