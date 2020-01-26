// Copyright (c) 2020
//
#ifndef DB_NODE_CACHE_MOCK_H_
#define DB_NODE_CACHE_MOCK_H_

#include <map>

namespace dbwheel {

class Node;

struct MockNodeCache: public NodeCache {

  Node* get(uint64_t pageID) override {
    auto i = cache.find(pageID);
    return i == cache.end() ? nullptr : i->second; 
  }

  void remove(uint64_t pageID) override {
    cache.erase(pageID);
  }
  std::map<uint64_t, Node*> cache;
};

}  // namespace dbwheel
#endif   // DB_NODE_CACHE_MOCK_H_
