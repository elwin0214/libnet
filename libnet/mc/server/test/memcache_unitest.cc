#include <gtest/gtest.h>
#include <libnet/mc/mem_cache.h>

using namespace std;
using namespace libnet;
using namespace mc::server;

TEST(Cache, get)
{
  ConcurrentSlabList slablist(SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024*1024*100)); 
  MemCache cache(8, 1.2, std::hash<std::string>(), slablist);
  function<size_t(string)> hash_func = hash<string>();
  const char* k = "abcdefg";
  const char* v = "1234567";
  Item* item = cache.alloc(7 + 7 + 2);
  item->set_key(k, 7);
  item->set_hashcode(hash_func(string(k, 7)));
  item->set_value(v, 7);
  cache.add(item);
  ASSERT_TRUE(NULL != item);
  Item* ritem = cache.find(k, true);
  ASSERT_TRUE(NULL != ritem);

  ASSERT_STREQ(k, ritem->key());
  ASSERT_STREQ(v, ritem->value());
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}