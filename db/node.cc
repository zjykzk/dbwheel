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

class InodeComp {
 public:
  bool operator() (const std::shared_ptr<inode> i, const std::string& key) {
    return i->key < key;
  }
};

void Node::put(std::string& oldKey, std::string& newKey, std::string& value, pgid id, uint32_t flags) {

  ASSERTM(oldKey.size()>0, "old key cannot be empty");
  ASSERTM(newKey.size()>0, "new key cannot be empty");

  auto pos = lower_bound(inodes_.begin(), inodes_.end(), oldKey, InodeComp());

  if (pos == inodes_.end()) {
    inodes_.insert(pos, std::shared_ptr<inode>(new inode{flags, id, newKey, value}));
  }
}

void Node::del(std::string& key) {

  auto pos = lower_bound(inodes_.begin(), inodes_.end(), key, InodeComp());

  if (pos != inodes_.end()) {
    inodes_.erase(pos);
  }
}

}  // namespace dbwheel
