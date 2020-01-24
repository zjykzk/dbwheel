// Copyright (c) 2020
//
#include "db/node.h"
#include "gtest/gtest.h"

namespace dbwheel {

using std::string;

TEST(NodeTest, putAndDel) {

  Node node;

  string oldKey = "old";
  string newKey = "new";
  string value = "value";
  node.put(oldKey, newKey, value, (pgid)1, 1);
  EXPECT_EQ(1, node.count());

  newKey = "other new";
  node.put(oldKey, newKey, value, (pgid)1, 1);

  EXPECT_EQ(2, node.count());

  node.del(newKey);
  EXPECT_EQ(1, node.count());
}

}  // namespace dbwheel
