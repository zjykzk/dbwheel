// Copyright (c) 2020
//
#include "db/node.h"
#include <cstring>

#include <algorithm>
#include <iterator>
#include <sstream>

#include "db/assert.h"
#include "db/debug.h"
#include "db/page_ele.h"
#include "db/inode.h"

namespace dbwheel {

struct InodeComp {
  bool operator() (const inode* i, const string& key) {
    return i->key < key;
  }
};

static inline void releaseMemOfNode(const vector<Node*>& nodes) {
  for (auto n : nodes) {
    delete n;
  }
}

Node::~Node() {

  for(auto i : inodes_) {
    delete i;
  }

  releaseMemOfNode(children_);
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

  auto i = del0(key);
  bool ok = i != nullptr;
  if (ok) {
    delete i;
  }

  return ok;
}

inode* Node::del0(const string& key) {

  inode* i = nullptr;
  auto pos = lower_bound(inodes_.begin(), inodes_.end(), key, InodeComp());
  if (pos != inodes_.end() && (*pos)->key == key) {
    i = *pos;
    inodes_.erase(pos);
  }

  return i;
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
  pageID_ = page->id();

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
  page->id(pageID_);

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

void Node::reblance(size_t pageSize, NodeCache& nodeCache, PageFree& pageFree) {

  if (sizeInPage() > pageSize/4 && inodes_.size() > minKeys()) {
    return;
  }

  if (parent_ == nullptr) {
    if (!isLeaf_ && inodes_.size() == 1) {
      collapse(nodeCache, pageFree);
    }
    return;
  }

  Node* toBeMerged;
  Node* target;
  vector<Node*>::iterator it;

  if (inodes_.empty()) {
    toBeMerged = this;
    goto FREE;
  }

  it = std::find(parent_->children_.begin(), parent_->children_.end(), this);
  ASSERTM(it != parent_->children_.end(), "BUG: current node not in the parent's children list");

  if (it == parent_->children_.begin()) {
    target = this;
    toBeMerged = *(it + 1);
  } else {
    target = *(it - 1);
    toBeMerged = this;
  }

  for (auto i : toBeMerged->inodes_) {
    auto n = nodeCache.get(i->pageID);
    if (n == nullptr) {
      continue;
    }

    n->parent_->removeChild(n);
    n->parent_ = target;
    target->children_.push_back(n);
  }

  target->inodes_.insert(target->inodes_.end(), toBeMerged->inodes_.begin(), toBeMerged->inodes_.end());

FREE:
  parent_->del0(toBeMerged->key());
  parent_->removeChild(toBeMerged);
  nodeCache.remove(toBeMerged->pageID_);
  pageFree.free(toBeMerged->pageID_);
  toBeMerged->inodes_.clear();

  // prevent field parent_'s value to being chaos since delete this when toBeMerged == this
  Node* parent = parent_;
  delete toBeMerged;

  parent->reblance(pageSize, nodeCache, pageFree);
}

void Node::removeChild(Node* n) {
  children_.erase(std::remove(children_.begin(), children_.end(), n));
}

size_t Node::sizeInPage() {

  size_t s = Page::kPageHeaderSize;
  size_t elsz = elementSize();

  for (auto i : inodes_) {
    s += elsz + inodeSizeInPage(i);
  }

  return s;
}

void Node::collapse(NodeCache& nodeCache, PageFree& pageFree) {

  Node* child = nodeCache.get(inodes_[0]->pageID);
  ASSERTM(child != nullptr, "child is null");

  isLeaf_ = child->isLeaf_;
  inodes_.swap(child->inodes_);
  children_.swap(child->children_);

  for (auto i : inodes_) {
    auto n = nodeCache.get(i->pageID);
    if (n != nullptr) {
      n->parent_ = this;
    }
  }

  // avoid delete self twice
  child->children_.clear();
  nodeCache.remove(child->pageID_);
  pageFree.free(child->pageID_);
  delete child;
}

inline const string& Node::key() const {
  return key_ == "" ? inodes_[0]->key : key_;
}

Node* Node::spill(size_t pageSize, double fillPercent, PageFree& pageFree, PageAlloc& pageAlloc) {

  if (spilled_) {
    return this;
  }

  std::sort(
      children_.begin(), children_.end(),
      [](Node* c1, Node* c2) { return c1->key() < c2->key(); });

  // cannot use loop, since the children can be modified since the split-merge operation
  for (size_t i = 0; i < children_.size(); i++) {
    auto c = children_[i];
    c->spill(pageSize, fillPercent, pageFree, pageAlloc);
  }

  // clear children
  children_.clear();

  for (auto n : split(pageSize, fillPercent)) {
    if (n->pageID_ > 0) {
      pageFree.free(n->pageID_);
    }

    Page* page = pageAlloc.alloc(pageSize, sizeInPage() / pageSize + 1);
    
    n->pageID_ = page->id();
    n->writePage(page);
    n->spilled_ = true;

    if (n->parent_ != nullptr) {
      if (n->key_ == "") {
        n->key_ = n->inodes_[0]->key;
      }

      n->parent_->put(n->key_, n->inodes_[0]->key, "", n->pageID_, 0);
    }
  }

  // root node spilted, so new root generated, do store it
  if (parent_ != nullptr && parent_->pageID_ == 0) {
    // avoid spill children again
    parent_->children_.clear();
    return parent_->spill(pageSize, fillPercent, pageFree, pageAlloc);
  }

  return this;
}

const string Node::toString() {

  std::stringstream s;
  s << "pageID:[" << pageID_ << "],is leaf:[" << isLeaf_ << "], inodes:[";
  for (auto i : inodes_) {
    s << "key=" << i->key << ",value=" << i->value << ",";
  }
  s << "],children count:[" << children_.size() << "]";

  return s.str();
}

}  // namespace dbwheel
