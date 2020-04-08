// Copyright (c) 2020
//
#ifndef DB_NODE_H_
#define DB_NODE_H_

#include <string>
#include <utility>
#include <vector>

#include "db/node_cache.h"
#include "db/page.h"
#include "db/page_alloc.h"
#include "db/page_free.h"

namespace dbwheel {

using std::string;
using std::vector;

struct inode;

class Node {
 public:
  Node(): Node(nullptr, 0, false) {}
  Node(Node* parent, uint64_t pageID, bool isLeaf): parent_(parent), pageID_(pageID), isLeaf_(isLeaf) {}
  Node(const vector<inode*>& inodes, bool isLeaf): parent_(nullptr), inodes_(inodes), isLeaf_(isLeaf) {}
  ~Node();

  vector<Node*> split(size_t pageSize, double fillPercent);
  void put(const string& oldKey, const string& newKey, const string& value, uint64_t id, uint32_t flags);
  bool del(const string& key);
  void readPage(Page* page);
  void writePage(Page* page);
  void reblance(size_t pageSize, NodeCache& nodeCache, PageFree& pageFree);
  Node* spill(size_t pageSize, double fillPercent, PageFree& pageFree, PageAlloc& pageAlloc);

  const bool isLeaf() const { return isLeaf_; }
  const int count() const { return inodes_.size(); }
  const uint64_t pageID() const { return pageID_; }
  const Node* const parent() const { return parent_; }
  const vector<inode*>& inodes() const { return inodes_; }
  const vector<Node*>& children() const { return children_; }
  void children(const vector<Node*>& children) { children_ = children; }

 private:
  std::pair<Node*, Node*> splitTwo(size_t pageSize, double fillPercent);
  bool sizeLessThan(size_t v);
  size_t elementSize();
  size_t splitIndex(size_t threshold);
  size_t sizeInPage();
  void writeLeaf(Page* page);
  void writeBranch(Page* page);
  void collapse(NodeCache& nodeCache, PageFree& pageFree);
  void removeChild(Node* n);
  Node* prevSilbing();
  Node* nextSilbing();
  const string& key() const;
  inode* del0(const string& key);

  size_t minKeys() { return isLeaf_ ? 1 : 2; }

  const string toString();

  Node* parent_;
  uint64_t pageID_;
  // children cache, used by spilling
  vector<Node*> children_;
  vector<inode*> inodes_;
  bool isLeaf_;
  string key_;
};

}  // namespace dbwheel

#endif  // DB_NODE_H_
