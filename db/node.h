// Copyright (c) 2020
//
#ifndef DB_NODE_H_
#define DB_NODE_H_

#include <string>
#include <vector>

#include "db/page.h"

namespace dbwheel {

struct inode;

class Node {
 private:
  std::vector<inode*> split(int pageSize);
  void put(std::string& oldKey, std::string& newKey, pgid id, uint32_t flags);

  Node* parent_;
  std::vector<std::unique_ptr<Node> > children_;
  std::vector<std::unique_ptr<inode> > inodes_;
};

}  // namespace dbwheel

#endif  // DB_NODE_H_
