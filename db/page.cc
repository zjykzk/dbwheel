// Copyright (c) 2020
//

#include "db/page.h"
#include <cstddef>

#include <sstream>

#include "db/page_ele.h"

namespace dbwheel {

static ssize_t kLeafPageElementSize = sizeof(leafPageElement);
static ssize_t kBranchPageElementSize = sizeof(branchPageElement);

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

const size_t Page::kPageHeaderSize = offsetof(Page, ptr_);
const size_t Page::kBranchPageElementSize = sizeof(branchPageElement);
const size_t Page::kLeafPageElementSize = sizeof(leafPageElement);
const size_t Page::kMinKeys = 2;

}  // namespace dbwheel
