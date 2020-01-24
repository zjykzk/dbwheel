// Copyright (c) 2020
//
#ifndef DB_NODE_H_
#define DB_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "db/page.h"

namespace dbwheel {

struct inode;

class Node {
 public:
  std::vector<inode*> split(int pageSize);
  void put(std::string& oldKey, std::string& newKey, std::string& value, pgid id, uint32_t flags);
  void del(std::string& key);
  int count() { return inodes_.size(); }

 private:
  std::weak_ptr<Node> parent_;
  std::vector<std::shared_ptr<Node> > children_;
  std::vector<std::shared_ptr<inode> > inodes_;
};

}  // namespace dbwheel

#endif  // DB_NODE_H_
