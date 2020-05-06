// Copyright (c) 2020
//
#ifndef DBWHEEL_DB_TX_IMPL_H_
#define DBWHEEL_DB_TX_IMPL_H_

#include <string>

#include "include/dbwheel/tx.h"

namespace dbwheel {

class Bucket;

class TXImpl : public TX {
 public:
  Bucket* createBucket(const std::string& name) override;
};

}  // namespace dbwheel

#endif  // DBWHEEL_DB_TX_IMPL_H_

