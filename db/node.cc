// Copyright (c) 2020
//
#include "db/node.h"
#include <cstring>

#include <algorithm>
#include <iterator>

#include "db/assert.h"
#include "db/debug.h"
#include "db/page_ele.h"
#include "db/inode.h"

namespace dbwheel {

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

void Node::put(
    const string& oldKey,
    const string& newKey,
    const string& value,
    uint64_t pageID,
    uint32_t flags) {

  ASSERTM(oldKey.size()>0, "old key cannot be empty");
  ASSERTM(newKey.size()>0, "new key cannot be empty");

  auto pos = lower_bound(inodes_.begin(), inodes_.end(), oldKey, InodeComp());

  if (pos == inodes_.end() || (*pos)->key != oldKey) {
    inodes_.insert(pos, new inode{flags, pageID, newKey, value});
    return;
  }

  auto n = *pos;
  n->pageID = pageID;
  n->flags = flags;
  n->key = newKey;
  n->value = value;
}

bool Node::del(const string& key) {

  auto pos = lower_bound(inodes_.begin(), inodes_.end(), key, InodeComp());

  auto ok = pos != inodes_.end();
  if (ok) {
    inodes_.erase(pos);
  }

  return ok;
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

static size_t inodeSizeInPage(inode* i) {
  return i->key.size() + i->value.size();
}

bool Node::sizeLessThan(size_t v) {

  size_t s = Page::kPageHeaderSize;
  size_t elsz = elementSize();

  for (auto i : inodes_) {
    s += elsz + inodeSizeInPage(i);
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
    sz += elsz + inodeSizeInPage(inodes_[i]);

    if (i >= Page::kMinKeys && sz > threshold) {
      break;
    }
  }

  return i;
}

void Node::readPage(Page* page) {

  isLeaf_ = (page->flags() & Page::kLeafPageFlag) > 0;
  pageId_ = page->id();

  uint32_t c = page->count();
  inodes_.reserve(c);

  if (isLeaf_) {
    auto e = page->leafPageElements();
    for (int i = 0; i < c; i++, e++) {
      inodes_.push_back(new inode{e->flags, page->id(), e->key(), e->value()});
    }
    return;
  }
  auto e = page->branchPageElements();
  for (int i = 0; i < c; i++, e++) {
    inodes_.push_back(new inode{0/*ignore*/, e->pageID, e->key()});
  }
}

void Node::writePage(Page* page) {

  page->flags(isLeaf_ ? Page::kLeafPageFlag : Page::kBranchPageFlag);
  page->id(pageId_);

  int inodeCount = inodes_.size();
  ASSERTM(inodeCount < 0xFFFF, "inode count overflow");

  page->count((uint32_t) inodeCount);

  if (inodeCount <= 0) {
    return;
  }

  if (isLeaf_) {
    writeLeaf(page);
  } else {
    writeBranch(page);
  }
}

void Node::writeLeaf(Page* page) {

  int inodeCount = inodes_.size();
  leafPageElement* elt = page->leafPageElements();
  char* kvData = reinterpret_cast<char*>(elt) + inodeCount * Page::kLeafPageElementSize;
  for (int i = 0; i < inodeCount; i++) {
    inode* in = inodes_[i];
    elt->flags = in->flags;
    elt->ksize = in->key.size();
    elt->vsize = in->value.size();
    elt->pos = (uint32_t)(kvData - reinterpret_cast<char*>(elt));

    memcpy(kvData, in->key.data(), elt->ksize);
    kvData += elt->ksize;
    memcpy(kvData, in->value.data(), elt->vsize);
    kvData += elt->vsize;

    elt++;
  }
}

void Node::writeBranch(Page* page) {

  int inodeCount = inodes_.size();
  branchPageElement* elt = page->branchPageElements();
  char* keyData = reinterpret_cast<char*>(elt) + inodeCount * Page::kBranchPageElementSize;
  for (int i = 0; i < inodeCount; i++) {
    inode* in = inodes_[i];
    elt->ksize = in->key.size();
    elt->pos = (uint32_t)(keyData - reinterpret_cast<char*>(elt));
    elt->pageID = in->pageID;

    memcpy(keyData, in->key.data(), elt->ksize);
    keyData += elt->ksize;

    elt++;
  }
}

}  // namespace dbwheel
