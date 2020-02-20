// Copyright (c) 2020
//
#ifndef DB_BUCKET_IMPL_H_
#define DB_BUCKET_IMPL_H_

#include <cstdint>
#include <string>

#include "include/dbwheel/bucket.h"

namespace dbwheel {

// bucket represents the on-file representation of a bucket.
// it's stored as the "value" of a bucket key. If the bucket is small enough,
// then its root page can be stored inline in the "value", after the bucker header.
// In the case of inline bucket, the "rootPageID" will be 0.
struct bucket {
  uint64_t rootPageID;
  uint64_t sequence; // monotonically incrementing
};

class BucketImpl : public Bucket {
 public:
  void put(const std::string& k, const std::string& v) override;
  void get(const std::string& k) override;
  void del(const std::string& k) override;

 private:
};

}  // namespace dbwheel

#endif  // DB_BUCKET_IMPL_H
