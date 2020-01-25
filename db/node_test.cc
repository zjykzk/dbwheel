// Copyright (c) 2020
//
#include "gtest/gtest.h"

#include "db/node.h"
#include "db/page.h"
#include "db/inode.h"

namespace dbwheel {

using std::string;

TEST(NodeTest, putAndDel) {

  Node node;

  string oldKey = "old";
  string newKey = "new";
  string value = "value";

  // insert "new"
  node.put(oldKey, newKey, value, 1, 1);
  EXPECT_EQ(1, node.count());

  // replace "new" with "old"
  node.put(newKey, oldKey, value, 1, 1);
  EXPECT_EQ(1, node.count());

  // insert "other new"
  newKey = "other new";
  node.put("new old", newKey, value, 1, 1);

  EXPECT_EQ(2, node.count());

  node.del(oldKey);
  EXPECT_EQ(1, node.count());
}

TEST(NodeTest, split) {

  string key10(10, 'k'), value10(10, 'v');
  string key20(20, 'k'), value20(20, 'v');
  Node node;
  node.put("0", key10, value10, 1, 1);
  node.put("0", key20, value20, 1, 1);

  // node count less than min keys
  vector<Node*> nodes = node.split(10, .5);
  ASSERT_EQ(1, nodes.size());
  ASSERT_EQ(&node, nodes[0]);
  ASSERT_EQ(nullptr, nodes[0]->parent());

  // not reach the fill percent
  string key30(30, 'k'), value30(30, 'v');
  string key40(40, 'k'), value40(40, 'v');
  node.put("0", key30, value30, 1, 1);
  node.put("0", key40, value40, 1, 1);
  nodes = node.split(400, .5);
  ASSERT_EQ(1, nodes.size());
  ASSERT_EQ(&node, nodes[0]);
  ASSERT_EQ(nullptr, nodes[0]->parent());

  // split now
  nodes = node.split(200, .5);
  ASSERT_EQ(2, nodes.size());
  ASSERT_EQ(&node, nodes[0]);
  ASSERT_TRUE(nullptr != nodes[0]->parent());
  ASSERT_EQ(2, node.parent()->children().size());
}

TEST(NodeTest, readWritePage) {

  char buf[1<<20];
  Page* page = reinterpret_cast<Page*>(buf);

  // empty inode
  {
    Node node1, node2;
    node1.writePage(page);
    node2.readPage(page);
    ASSERT_EQ(0, node2.count());
  }

  string k(100, 'k'), v(100, 'v');
  string k2(200, 'k'), v2(200, 'v');

  // some kv branch
  {
    Node node1, node2;
    node1.put(k, k, v, 1, 1);
    node1.put(k2, k2, v2, 2, 2);
    node1.writePage(page);
    node2.readPage(page);
    ASSERT_EQ(2, node2.count());
    vector<inode*> ins = node2.inodes();
    inode* in1 = ins[0];
    ASSERT_EQ(1, in1->pageID);
    ASSERT_EQ(k, in1->key);

    inode* in2 = ins[1];
    ASSERT_EQ(2, in2->pageID);
    ASSERT_EQ(k2, in2->key);
  }

  // some kv leaf
  {
    Node node1(vector<inode*>(), true), node2(vector<inode*>(), true);
    node1.put(k, k, v, 1, 1);
    node1.put(k2, k2, v2, 2, 2);
    node1.writePage(page);
    node2.readPage(page);

    ASSERT_EQ(2, node2.count());
    vector<inode*> ins = node2.inodes();
    inode* in1 = ins[0];
    ASSERT_EQ(1, in1->flags);
    ASSERT_EQ(k, in1->key);
    ASSERT_EQ(v, in1->value);

    inode* in2 = ins[1];
    ASSERT_EQ(2, in2->flags);
    ASSERT_EQ(k2, in2->key);
    ASSERT_EQ(v2, in2->value);
  }
}

}  // namespace dbwheel
