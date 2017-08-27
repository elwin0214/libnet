#include <iostream>
#include <libnet/logger.h>
#include <gtest/gtest.h>
#include <libnet/mc/htable.h>
#include <libnet/mc/item.h>
#include <libnet/mc/slab.h>

using namespace mc::server;
using namespace std;

function<size_t(string)> gHash = hash<string>();
struct HTableWrapper
{
  HTableWrapper(size_t hashpower, double factor)
    : htable_(hashpower, factor)
  {
    htable_.set_hash(gHash);
    htable_.set_equals([](const char* k1, const char* k2){
      return ::strcmp(k1, k2) == 0;
    });

    htable_.set_getkey([](Item* item){
      return item->key();
    });

    htable_.set_gethash([](Item* item){
      return item->hashcode();
    });
  }
  HTable<const char*, Item*> htable_;
};



TEST(HTable, get_set)
{
  SlabOption option(16, 32, 1.2, 1024, true, 1024 * 1024 * 2);
  SlabList slab_list(option);
  int index = -1;
  Item* item1 = slab_list.pop(16, index);
  Item* item2 = slab_list.pop(16, index);

  HTableWrapper hw(2, 1.2);
  HTable<const char*, Item*>& hashtable = hw.htable_;
  item1->set_key("1", 1);
  item1->set_hashcode(gHash("1"));
  item1->set_value("a", 1);

  item2->set_key("2", 1);
  item2->set_hashcode(gHash("2"));

  item2->set_value("b", 1);
  ASSERT_STREQ("2", item2->key());

  hashtable.setItem(item1);
  hashtable.setItem(item2);

  ASSERT_EQ(2, hashtable.size());
  Item* item = hashtable.get("1");
  ASSERT_TRUE(NULL != item);

  Item* remove_item = hashtable.remove("2");
  ASSERT_EQ(1, hashtable.size());
  ASSERT_STREQ("2", remove_item->key());
}

TEST(HTable, resize)
{
  SlabOption option(16, 32, 1.2, 1024, true, 1024 * 1024 * 2);
  SlabList slab_list(option);
  HTableWrapper hw(2, 1.2);
  HTable<const char*, Item*>& hashtable = hw.htable_;

  for (size_t index = 0; index < 2048; index++)
  {
    int item_index = -1;
    Item* item = slab_list.pop(16, item_index);
    std::string str = std::to_string(index);
    const char* key = str.c_str();
    item->set_key(key, ::strlen(key));
    item->set_hashcode(gHash(str));

    item->set_value(key, ::strlen(key));
    hashtable.setItem(item);
    cout << "moving = " << (hashtable.moving()) << " size = " << hashtable.size() << " buckets = " << hashtable.buckets() << endl;

    Item* get_item = hashtable.get(key);

    ASSERT_TRUE(NULL != get_item );
    cout << (get_item->key()) << " " << (get_item->value() ) << endl ;

    ASSERT_STREQ(key, get_item->key());
    ASSERT_STREQ(key, get_item->value());
    //assert(::strcmp(get_item->key(), key) == 0 && ::strcmp(get_item->value(), key) == 0);

    if (index % 3 == 0)
    {
      Item* remove_item = hashtable.remove(key);
      //assert(::strcmp(remove_item->key(), key) == 0 && ::strcmp(remove_item->value(), key) == 0);
      ASSERT_STREQ(key, remove_item->key());
      ASSERT_STREQ(key, remove_item->value());

      get_item = hashtable.get(key);
      ASSERT_TRUE(NULL == get_item);
    }
    
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}