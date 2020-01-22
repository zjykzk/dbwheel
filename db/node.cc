// Copyright (c) 2020
//
#include "db/node.h"
#include <algorithm>

#include "db/assert.h"

namespace dbwheel {

struct inode {
  uint32_t flags;
  pgid id;
  std::string key;
  std::string value;
};

void Node::put(std::string& oldKey, std::string& newKey, pgid id, uint32_t flags) {

  ASSERTM(oldKey.size()>0, "old key cannot be empty");
  ASSERTM(newKey.size()>0, "new key cannot be empty");

  auto pos = find_if(
      inodes_.begin(),
      inodes_.end(),
      [&oldKey](inode* i) { return i->key >= oldKey; });

  if (pos == inodes_.end()) {
    inodes_.insert(pos, new inode{});
  }
}

}
