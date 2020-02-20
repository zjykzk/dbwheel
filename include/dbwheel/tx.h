// Copyright (c) 2020
//
#ifndef DBWHEEL_INCLUDE_TX_H_
#define DBWHEEL_INCLUDE_TX_H_

#include <string>

namespace dbwheel {

class Bucket;

class TX {
 public:
  virtual Bucket* createBucket(const std::string& name) = 0;
};

}  // namespace dbwheel

#endif  // DBWHEEL_INCLUDE_TX_H

