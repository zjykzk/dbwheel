// Copyright (c) 2020
//
#ifndef DB_INODE_H_
#define DB_INODE_H_

namespace dbwheel {

struct inode {
  uint32_t flags;
  uint64_t pageID;
  string key;
  string value;
};

}  // namespace dbwheel

#endif  // DB_INODE_H
