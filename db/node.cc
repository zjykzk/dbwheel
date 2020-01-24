// Copyright (c) 2020
//
#include "db/node.h"
#include <algorithm>

#include "db/assert.h"
#include "db/debug.h"

namespace dbwheel {

struct inode {
  uint32_t flags;
  pgid id;
  string key;
  string value;

  size_t sizeInPage() {
    return key.size() + value.size();
  }
};

class InodeComp {
 public:
  bool operator() (const inode* i, const string& key) {
    return i->key < key;
  }
};

Node::~Node() {

  for(auto i : inodes_) {
    delete i;
  }

  for (auto n : children_) {
    delete n;
  }
}

void Node::put(const string& oldKey, const string& newKey, const string& value, pgid id, uint32_t flags) {

  ASSERTM(oldKey.size()>0, "old key cannot be empty");
  ASSERTM(newKey.size()>0, "new key cannot be empty");

  auto pos = lower_bound(inodes_.begin(), inodes_.end(), oldKey, InodeComp());

  if (pos == inodes_.end() || (*pos)->key != oldKey) {
    inodes_.insert(pos, new inode{flags, id, newKey, value});
    return;
  }

  auto n = *pos;
  n->id = id;
  n->flags = flags;
  n->key = newKey;
  n->value = value;
}

void Node::del(const string& key) {

  auto pos = lower_bound(inodes_.begin(), inodes_.end(), key, InodeComp());

  if (pos != inodes_.end()) {
    inodes_.erase(pos);
  }
}

vector<Node*> Node::split(size_t pageSize, double fillPercent) {

  vector<Node*> nodes;
  Node* node = this;

  while (true) {

    auto np = node->splitTwo(pageSize, fillPercent);
    Node* a = std::get<0>(np);
    Node* b = std::get<1>(np);
    nodes.push_back(a);

    if (b == nullptr) {
      break;
    }

    node = b;
  }

  return nodes;
}

std::pair<Node*, Node*> Node::splitTwo(size_t pageSize, double fillPercent) {

  if (inodes_.size() < Page::kMinKeys * 2 || sizeLessThan(pageSize)) {
    return std::make_pair(this, nullptr);
  }

  auto i = splitIndex((size_t) (fillPercent * pageSize));

  if (parent_ == nullptr) {
    parent_ = new Node();
    parent_->children_.push_back(this);
  }

  Node* n = new Node(
      vector<inode*>(inodes_.begin() + i, inodes_.end()),
      isLeaf_);
  n->parent_ = parent_;
  parent_->children_.push_back(n);

  inodes_.erase(inodes_.begin() + i, inodes_.end());

  return std::make_pair(this, n);
}

bool Node::sizeLessThan(size_t v) {

  size_t s = Page::kPageHeaderSize;
  size_t elsz = elementSize();

  for (auto i : inodes_) {
    s += elsz + i->sizeInPage();
    if (s >= v) {
      return false;
    }
  }

  return true;
}

inline size_t Node::elementSize() {

  if (isLeaf_) {
    return Page::kLeafPageElementSize;
  }

  return Page::kBranchPageElementSize;
}


size_t Node::splitIndex(size_t threshold) {

  size_t i = 0, sz = Page::kPageHeaderSize, elsz = elementSize();
  for (size_t s = inodes_.size(); i < s; i++) {
    sz += elsz + inodes_[i]->sizeInPage();

    if (i >= Page::kMinKeys && sz > threshold) {
      break;
    }
  }

  return i;
}

}  // namespace dbwheel
