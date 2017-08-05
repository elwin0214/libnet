#include <iostream>
#include <libnet/logger.h>
#include <gtest/gtest.h>
#include <libnet/mc/slab.h>

using namespace std;
using namespace libnet;
using namespace mc::server;

TEST(Slab, prealloc_slab)
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  SlabOption option = {16, 1024, 1.2, 1024, true, 1024 * 1024 * 2};
  SlabArray slab_array(option);
  //slab_array.init();
  int item_index = -1;
  Item* item = slab_array.pop(16, item_index);
  ASSERT_TRUE(NULL != item);
  ASSERT_EQ(item->size(), 16);
  const char* k = "abcdefg";
  const char* v = "1234567";

  item->set_key(k, 7);
  item->set_value(v, 7);

  ASSERT_STREQ(k, item->key());
  ASSERT_STREQ(v, item->value());
}

TEST(Slab, slab_number)
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  SlabOption option = {16, 1024, 1.2, 1024, true, 1024 * 1024 * 2};
  SlabArray slab_array(option);

  size_t slabs = slab_array.slabs();
  for (size_t i = 0 ; i < slabs; i++)
  {
    Slab& slab = slab_array[i];
    cout << "index = " << (slab.index()) <<"item size = " << (slab.item_size()) <<" number = " << (slab.number()) << endl; 
  }
}

TEST(Slab, pop_push)
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  SlabOption option = {16, 1024, 1.2, 1024, true, 1024 * 1024 * 2};
  SlabArray slab_array(option);

  Slab& slab = slab_array[0];

  std::vector<Item*> items;
  int item_index = -1;
  Item* item = slab_array.pop(16, item_index); //index = 0

  ASSERT_TRUE(NULL != item);
  items.push_back(item);// first 
  size_t number = slab.number();

  for (int i = 0; i < number; i++)
  {
    ASSERT_EQ(number - i, slab.number());
    Item* item = slab.pop();
    ASSERT_TRUE(NULL != item);

    items.push_back(item);
  }
  ASSERT_EQ(0, slab.number());

  int index = 0;
  for (auto item : items)
  {
    index++;
    slab.push(item);
    ASSERT_EQ(index, slab.number());
  }

  ASSERT_EQ(number + 1, slab.number());
}
 
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}