#ifndef __LIBNET_MEMCACHED_MEMCACHE_H__
#define __LIBNET_MEMCACHED_MEMCACHE_H__
#include <libnet/nocopyable.h>
#include "slab.h"
#include "hashtable.h"
#include "lru.h"

namespace memcached
{
namespace server
{
class MemCache : public libnet::NoCopyable
{
public:
  MemCache()
    : hash_table_(8, 1.2),
      slab_array_(SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 64)), //64M
      lru_list_(slab_array_.slabs())
  {

  }

  MemCache(size_t hashpower, double factor, const SlabOption& slab_options)
    : hash_table_(hashpower, factor),
      slab_array_(slab_options),
      lru_list_(slab_array_.slabs())
  {

  }
  
  void remove(Item* item);
  Item* find(const char* key, bool lru);
  Item* alloc(size_t data_size); 
  void add(Item* item);

private:
  HashTable hash_table_;
  SlabArray slab_array_;
  LRUList lru_list_;
};

}
}
#endif