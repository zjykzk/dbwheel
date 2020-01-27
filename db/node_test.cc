// Copyright (c) 2020
//
#include "gtest/gtest.h"

#include "db/inode.h"
#include "db/node.h"
#include "db/node_cache_mock.h"
#include "db/page.h"
#include "db/page_alloc_mock.h"
#include "db/page_free_mock.h"

namespace dbwheel {

using std::string;

TEST(TestNode, putAndDel) {

  Node node;
  string value = "v";

  node.put("1", "1", value, 1, 1);
  EXPECT_EQ(1, node.count());

  node.put("1", "2", value, 1, 1);
  EXPECT_EQ(1, node.count());
  auto in = node.inodes()[0];
  ASSERT_EQ("2", in->key);

  node.put("3", "3", value, 1, 1);
  EXPECT_EQ(2, node.count());
  auto in1 = node.inodes()[0];
  auto in2 = node.inodes()[1];
  ASSERT_EQ("2", in1->key);
  ASSERT_EQ("3", in2->key);

  node.del("4");
  EXPECT_EQ(2, node.count());

  node.del("2");
  EXPECT_EQ(1, node.count());
}

TEST(TestNode, split) {

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

TEST(TestNode, readWritePage) {

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

TEST(TestNode, noReblance) {

  MockPageFree pageFree;
  MockNodeCache nodeCache;

  Node b;
  b.put("1", "1", "1", 1, 1);
  b.put("2", "2", "2", 1, 1);
  b.put("3", "3", "3", 1, 1);
  b.reblance(10000, nodeCache, pageFree);

  ASSERT_EQ(nullptr, b.parent());
  ASSERT_EQ(3, b.count());

  Node l({},  true);
  l.put("1", "1", "1", 1, 1);
  l.reblance(10000, nodeCache, pageFree);
}

TEST(TestNode, reblanceCollapse) {

  MockPageFree pageFree;
  MockNodeCache nodeCache;
  Node b;
  b.put("1", "1", "1", 1, 1);

  Node* b1 = new Node(&b, 1, true);
  b1->put("2", "2", "2v", 2, 1);
  b1->put("3", "3", "3v", 3, 1);

  nodeCache.cache[1] = b1;

  b.reblance(1, nodeCache, pageFree);

  auto ins = b.inodes();
  ASSERT_EQ(2, ins.size());
  auto i1 = ins[0];
  ASSERT_EQ("2", i1->key);
  ASSERT_EQ("2v", i1->value);
  auto i2 = ins[1];
  ASSERT_EQ("3", i2->key);
  ASSERT_EQ("3v", i2->value);

  ASSERT_EQ(0, nodeCache.cache.count(1));
  ASSERT_EQ(1, pageFree.freed[0]);

  ASSERT_TRUE(b.isLeaf());
}

TEST(TestNode, reblanceMergeToPrev) {

  MockPageFree pageFree;
  MockNodeCache nodeCache;
  Node b;
  b.put("1", "1", "1", 1, 1);
  b.put("2", "2", "2", 2, 1);

  Node* b1 = new Node(&b, 1, true);
  b1->put("11", "11", "11v", 3, 1);

  Node* b2 = new Node(&b, 2, true);
  b2->put("2", "2", "21v", 4, 1);

  b.children({b1, b2});

  nodeCache.cache[1] = b1;
  nodeCache.cache[2] = b2;

  b2->reblance(10000, nodeCache, pageFree);

  ASSERT_TRUE(b.isLeaf());
  ASSERT_EQ(0, b.children().size());

  auto ins = b.inodes();
  ASSERT_EQ(2, ins.size());
  auto i1 = ins[0];
  ASSERT_EQ("11", i1->key);
  ASSERT_EQ("11v", i1->value);
  auto i2 = ins[1];
  ASSERT_EQ("2", i2->key);
  ASSERT_EQ("21v", i2->value);

  ASSERT_EQ(0, nodeCache.cache.count(1));
  ASSERT_EQ(0, nodeCache.cache.count(2));
  ASSERT_EQ(2, pageFree.freed[0]);
  ASSERT_EQ(1, pageFree.freed[1]);

}

TEST(TestNode, reblanceMergeToSelf) {

  MockPageFree pageFree;
  MockNodeCache nodeCache;
  Node b;
  b.put("1", "1", "1", 1, 1);
  b.put("2", "2", "2", 2, 1);

  Node* b1 = new Node(&b, 1, true);
  b1->put("11", "11", "11v", 3, 1);

  Node* b2 = new Node(&b, 2, true);
  b2->put("2", "2", "21v", 4, 1);

  b.children({b1, b2});

  nodeCache.cache[1] = b1;
  nodeCache.cache[2] = b2;

  b1->reblance(10000, nodeCache, pageFree);

  ASSERT_TRUE(b.isLeaf());
  ASSERT_EQ(0, b.children().size());

  auto ins = b.inodes();
  ASSERT_EQ(2, ins.size());
  auto i1 = ins[0];
  ASSERT_EQ("11", i1->key);
  ASSERT_EQ("11v", i1->value);
  auto i2 = ins[1];
  ASSERT_EQ("2", i2->key);
  ASSERT_EQ("21v", i2->value);

  ASSERT_EQ(0, nodeCache.cache.count(1));
  ASSERT_EQ(0, nodeCache.cache.count(2));
  ASSERT_EQ(2, pageFree.freed[0]);
  ASSERT_EQ(1, pageFree.freed[1]);

}

TEST(TestNode, spill) {

  Node* root = new Node(nullptr, 1, false);
  root->put("1", "1", "1", 2, 0);
  root->put("2", "2", "2", 3, 0);
  Node* l1 = new Node(root, 2, true);
  Node* l2 = new Node(root, 3, true);
  root->children({l1, l2});

  l1->put("1", "1", "1", 2, 0);
  l1->put("11", "11", "11", 2, 0);
  l1->put("12", "12", "12", 2, 0);
  l1->put("13", "13", "13", 2, 0);

  l2->put("2", "2", "2", 3, 0);
  l2->put("21", "21", "21", 3, 0);
  l2->put("22", "22", "22", 3, 0);
  l2->put("23", "23", "23", 3, 0);

  MockPageAlloc pageAlloc(4);
  MockPageFree pageFree;

  Node* newRoot = root->spill(40, 0.5, pageFree, pageAlloc);
  auto ins = newRoot->inodes();
  ASSERT_EQ(2, ins.size());
  auto in0 = ins[0];
  ASSERT_EQ("1", in0->key);
  ASSERT_EQ("", in0->value);
  ASSERT_EQ(8, in0->pageID);

  auto in1 = ins[1];
  ASSERT_EQ("2", in1->key);
  ASSERT_EQ("", in1->value);
  ASSERT_EQ(9, in1->pageID);

  auto p = pageAlloc.alloced[in0->pageID];
  Node n;
  n.readPage(p);
  auto ins1 = n.inodes();
  ASSERT_EQ(2, ins1.size());
  auto in10 = ins1[0];
  ASSERT_EQ("1", in10->key);
  ASSERT_EQ("", in10->value);
  ASSERT_EQ(4, in10->pageID);
  auto in11 = ins1[1];
  ASSERT_EQ("12", in11->key);
  ASSERT_EQ("", in11->value);
  ASSERT_EQ(5, in11->pageID);

  p = pageAlloc.alloced[in1->pageID];
  Node n2;
  n2.readPage(p);
  auto ins2 = n2.inodes();
  ASSERT_EQ(2, ins2.size());
  auto in20 = ins2[0];
  ASSERT_EQ("2", in20->key);
  ASSERT_EQ("", in20->value);
  ASSERT_EQ(6, in20->pageID);
  auto in21 = ins2[1];
  ASSERT_EQ("22", in21->key);
  ASSERT_EQ("", in21->value);
  ASSERT_EQ(7, in21->pageID);

  p = pageAlloc.alloced[in21->pageID];
  Node n3;
  n3.readPage(p);
  ASSERT_TRUE(n3.isLeaf());
  auto ins3 = n3.inodes();
  ASSERT_EQ(2, ins3.size());
  auto in30 = ins3[0];
  ASSERT_EQ("22", in30->key);
  ASSERT_EQ("22", in30->value);
  ASSERT_EQ(in21->pageID, in30->pageID);
  auto in31 = ins3[1];
  ASSERT_EQ("23", in31->key);
  ASSERT_EQ("23", in31->value);
  ASSERT_EQ(in21->pageID, in31->pageID);
}

}  // namespace dbwheel
