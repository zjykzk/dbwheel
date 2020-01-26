// Copyright (c) 2020
//
#ifndef DB_NODE_CACHE_H_
#define DB_NODE_CACHE_H_

namespace dbwheel {

class Node;

class NodeCache {
 public:
  virtual Node* get(uint64_t pageID) = 0;
  virtual void remove(uint64_t pageID) = 0;
};

}  // namespace dbwheel

#endif  // DB_NODE_CACHE_H_
