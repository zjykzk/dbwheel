// Copyright (c) 2020
//
#ifndef DBWHEEL_INCLUDE_BUCKET_H_
#define DBWHEEL_INCLUDE_BUCKET_H_

#include <string>

namespace dbwheel {

class Bucket {
 public:
  virtual void put(const std::string& k, const std::string& v) = 0;
  virtual void get(const std::string& k) = 0;
  virtual void del(const std::string& k) = 0;
};

}  // namespace dbwheel

#endif  // DBWHEEL_INCLUDE_BUCKET_H
