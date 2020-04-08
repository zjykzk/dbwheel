// Copyright (c) 2020
//
#ifndef DBWHEEL_INCLUDE_BUCKET_H_
#define DBWHEEL_INCLUDE_BUCKET_H_

#include <string>

#include "include/dbwheel/status.h"

namespace dbwheel {

class Bucket {
 public:
  virtual Status put(const std::string& k, const std::string& v) = 0;
  virtual Status get(const std::string& k, std::string* v) = 0;
  virtual Status del(const std::string& k) = 0;
};

}  // namespace dbwheel

#endif  // DBWHEEL_INCLUDE_BUCKET_H
