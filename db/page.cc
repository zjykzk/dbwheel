// Copyright (c) 2020
//

#include "db/page.h"

#include <sstream>

namespace dbwheel {


struct branchPageElement {
  std::string key();

  uint32_t pos;
  uint32_t ksize;
  pgid pgid_;
};

std::string branchPageElement::key() {
  return std::string((reinterpret_cast<char*>(this) + this->pos), this->ksize);
}


struct leafPageElement {
  std::string key();
  std::string value();

  uint32_t flags;
  uint32_t pos;
  uint32_t ksize;
  uint32_t vsize;
};


std::string leafPageElement::key() {
  return std::string((reinterpret_cast<char*>(this) + this->pos), this->ksize);
}

std::string leafPageElement::value() {
  return std::string((reinterpret_cast<char*>(this) + this->pos + this->ksize), this->vsize);
}

static ssize_t kLeafPageElementSize = sizeof(leafPageElement);
static ssize_t kBranchPageElementSize = sizeof(branchPageElement);

enum {
  kBranchPageFlag = 0x01,
  kLeafPageFlag = 0x02,
  kMetaPageFlag = 0x04,
  kFreeListPageFlag = 0x10
};

const std::string Page::type() {
  if ((flags_ & kBranchPageFlag) != 0) {
    return "branch";
  }

  if ((flags_ & kLeafPageFlag) != 0) {
    return "leaf";
  }

  if ((flags_ & kMetaPageFlag) != 0) {
    return "meta";
  }

  if ((flags_ & kFreeListPageFlag) != 0) {
    return "freeList";
  }

  std::stringstream s;
  s << "unknow<" << flags_ << '>';
  return s.str();
}

branchPageElement* Page::branchPageElementOf(uint16_t index) {
  return reinterpret_cast<branchPageElement*>(this->ptr_) + index;
}

leafPageElement* Page::leafPageElementOf(uint16_t index) {
  return reinterpret_cast<leafPageElement*>(this->ptr_) + index;
}

}  // namespace dbwheel
