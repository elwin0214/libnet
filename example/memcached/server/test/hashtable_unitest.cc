#include <iostream>
#include <libnet/logger.h>
#include "assert.h"
#include "../hashtable.h"
#include "../item.h"
#include "../slab.h"

using namespace memcached::server;
using namespace std;

void test_set()
{
  SlabPolicy policy = {16, 32, 1.2, true, 1024 * 1024 * 2};
  SlabArray slab_array(policy);
  slab_array.init();

  Item* item1 = slab_array.pop(16);
  Item* item2 = slab_array.pop(16);

  HashTable ht(2, 1.2);
  item1->set_key("1", 1);
  item1->set_value("a", 1);

  item2->set_key("2", 1);
  item2->set_value("b", 1);
  cout << "item2.key = " << (item2->key()) << endl;
  assert(strcmp("2", item2->key()) == 0);

  ht.set(item1);
  ht.set(item2);

  assert(2 == ht.size());
  Item* item = ht.get("1");

  Item* remove_item = ht.remove("2");
  assert(1 == ht.size());
  assert(strcmp("2", remove_item->key()) == 0);


}

int main()
{
  test_set();
  return 0;
}