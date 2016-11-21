#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include "../lru.h"

using namespace std;
using namespace libnet;
using namespace memcached::server;

TEST(LRU, add_boundary)
{
  LRUList lru(2);
  Item* item = new Item(0, 2);
  ASSERT_EQ(lru.number(0), 0);
  lru.add(item);
  ASSERT_EQ(lru.number(0), 1);
  lru.remove(item);
  ASSERT_EQ(lru.number(0), 0);
}

TEST(LRU, add_batch)
{
  LRUList lru(2);

  for (int i = 0 ; i < 10; i++)
  {
    Item* item0 = new Item(0, i); //will not delete
    lru.add(item0);
    Item* item1 = new Item(1, i); //will not delete
    lru.add(item1);
  }

  int index = 9;
  Item* head0 = lru.head(0);
  Item* tail0 = lru.tail(0);
  while (head0 != NULL)
  {
    ASSERT_EQ (head0->size_, index);
    head0 = head0->next_;
    index--;
  }

  index = 0;
  while (tail0 != NULL)
  {
    ASSERT_EQ(tail0->size_, index);
    tail0 = tail0->prev_;
    index++;
  }
}

TEST(LRU, remove)
{
  LRUList lru(2);
  size_t sum = 100;
  for (int i = 0 ; i < sum; i++)
  {
    Item* item0 = new Item(0, i); //will not delete
    lru.add(item0);
    if (i % 5 == 0)
    {
      lru.remove(item0);
    }
    Item* item1 = new Item(1, i); //will not delete
    lru.add(item1);
    if (i % 5 == 2)
    {
      lru.remove(item1);
    }
  }

  Item* head0 = lru.head(0);
  Item* head1 = lru.head(1);
  for (int i = 99; i >= 0; i--)
  {
    if (i % 5 == 0) continue;
    ASSERT_EQ(head0->size_, i);
    head0 = head0->next_;
  }
  for (int i = 99; i >= 0; i--)
  {
    if (i % 5 == 2) continue;
    ASSERT_EQ(head1->size_, i);
    head1 = head1->next_;
  }
}

TEST(LRU, cycle)
{
  LRUList lru(2);
  size_t sum = 100;
  for (int i = 0 ; i < sum; i++)
  {
    // 990,980,...30,20,10,0
    Item* item0 = new Item(0, i); //will not delete
    item0->exptime_ = i * 10 ; 
    lru.add(item0);
    
    Item* item1 = new Item(1, i); //will not delete
    item1->exptime_ = 10000;
    lru.add(item1); 
  }

  Item* c1 = lru.recycle(0, 15);
  Item* c2 = lru.recycle(1, 15);
  ASSERT_TRUE(NULL != c1);
  ASSERT_EQ(c1->size_, 1);
  ASSERT_TRUE(NULL != c2);
  ASSERT_EQ(c2->size_, 0);
}
 
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}