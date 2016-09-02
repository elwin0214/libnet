#include <iostream>
#include <libnet/logger.h>
#include "assert.h"
#include "../slab.h"


using namespace std;
using namespace libnet;
using namespace memcached::server;

void test_prealloc_slab()
{
  SlabPolicy policy = {16, 32, 1.2, true, 1024 * 1024 * 2};
  SlabArray slab_array(policy);
  slab_array.init();
  Item* item = slab_array.pop(16);
  assert (NULL != item);
  assert (item->size() == 16);
  const char* k = "abcdefg";
  const char* v = "1234567";

  item->set_key(k, 7);
  item->set_value(v, 7);

  assert(::strcmp(k, item->key()) == 0);
  assert(::strcmp(v, item->value()) == 0);
  
}


void test_slab_number()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  SlabPolicy policy = {16, 128, 1.2, true, 1024 * 1024 * 2};
  SlabArray slab_array(policy);
  slab_array.init();

  size_t slabs = slab_array.slabs();
  for (size_t i = 0 ; i < slabs; i++)
  {
    Slab& slab = slab_array[i];
    cout << "index = " << (slab.index()) <<"item size = " << (slab.item_size()) <<" number = " << (slab.number()) << endl; 
  }
}

void test_slab_pop_push()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  SlabPolicy policy = {16, 128, 1.2, true, 1024 * 1024 * 2};
  SlabArray slab_array(policy);
  slab_array.init();

  Slab& slab = slab_array[0];

  std::vector<Item*> items;
  //Item* item = slab.pop();
  Item* item = slab_array.pop(16); //index = 0

  assert(NULL != item);
  items.push_back(item);// first 
  size_t number = slab.number();

  for (int i = 0; i < number; i++)
  {
    assert(number - i == slab.number());
    Item* item = slab.pop();
    assert(NULL != item);

    items.push_back(item);
  }
  assert(0 == slab.number());

  int index = 0;
  for (auto item : items)
  {
    index++;
    slab.push(item);
    assert(index == slab.number());
  }

  assert(number + 1 == slab.number());

}

int main()
{
  test_prealloc_slab();
  test_slab_number();
  test_slab_pop_push();
}