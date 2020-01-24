// Copyright (c) 2020
//
#ifndef DB_NODE_H_
#define DB_NODE_H_

#include <string>
#include <utility>
#include <vector>

#include "db/page.h"

namespace dbwheel {

using std::string;
using std::vector;

struct inode;

class Node {
 public:
  Node(): parent_(nullptr), isLeaf_(false) {}

  Node(const vector<inode*>& inodes, bool isLeaf): parent_(nullptr), inodes_(inodes), isLeaf_(isLeaf) {}
  ~Node();

  vector<Node*> split(size_t pageSize, double fillPercent);
  void put(const string& oldKey, const string& newKey, const string& value, pgid id, uint32_t flags);
  void del(const string& key);

  int count() { return inodes_.size(); }
  const Node* const parent() const { return parent_; }
  const vector<inode*> inodes() const { return inodes_; }
  const vector<Node*> children() const { return children_; }

 private:
  std::pair<Node*, Node*> splitTwo(size_t pageSize, double fillPercent);
  bool sizeLessThan(size_t v);
  size_t elementSize();
  size_t splitIndex(size_t threshold);

  Node* parent_;
  vector<Node*> children_;
  vector<inode*> inodes_;
  bool isLeaf_;
};

}  // namespace dbwheel

#endif  // DB_NODE_H_
