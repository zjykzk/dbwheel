// Copyright (c) 2020
//
#ifndef DB_INODE_H_
#define DB_INODE_H_

#include <string>

namespace dbwheel {

struct inode {
  uint32_t flags;
  uint64_t pageID;
  std::string key;
  std::string value;
};

}  // namespace dbwheel

#endif  // DB_INODE_H
