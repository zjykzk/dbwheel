// Copyright (c) 2020
//
#include "gtest/gtest.h"

#include "db/db_impl.h"
#include "db/debug.h"

namespace dbwheel {

TEST(TestDBImpl, open) {
  DB *db;
  Status status = DB::open(Options{}, "testOpen", &db);

  debug << status.toString() << std::endl;

  if (status.ok()) {
    db->close();
    delete db;
  }
}

}  // namespace dbwheel
