#ifndef __LIBNET_MC_MEMCACHE_H__
#define __LIBNET_MC_MEMCACHE_H__
#include <libnet/nocopyable.h>
#include <libnet/mc/slab.h>
#include <libnet/mc/htable.h>
#include <libnet/mc/lru.h>
#include <libnet/mc/item.h>

namespace mc
{
namespace server
{
class MemCache : public libnet::NoCopyable
{
public:
  typedef std::function<size_t(std::string)> HashFunc;

public:
  // MemCache(const ConcurrentSlabList& slablist_)
  //   : hash_table_(8, 1.2),
  //     slablist_(slablist_/*SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 64)*/), //64M
  //     lru_list_(slablist_.size())
  // {
  //   hash_table_.set_hash(std::hash<std::string>());
  //   Item::set_hash_func(std::hash<std::string>());
  //   hash_table_.set_equals([](const char* k1, const char* k2){
  //     return (0 == ::strcmp(k1, k2));
  //   });

  //   hash_table_.set_getkey([](Item* item){
  //     return item->key();
  //   });

  //   hash_table_.set_gethash([](Item* item){
  //     return item->hashcode();
  //   });
  // }

  MemCache(size_t hashpower, double factor, HashFunc hash_func, ConcurrentSlabList& slablist)
    : hash_table_(hashpower, factor),
      slablist_(slablist),
      lru_list_(slablist.size())
  {
    hash_table_.set_hash(hash_func);
    hash_table_.set_equals([](const char* k1, const char* k2){
      return (0 == ::strcmp(k1, k2));
    });

    hash_table_.set_getkey([](Item* item){
      return item->key();
    });

    hash_table_.set_gethash([](Item* item){
      return item->hashcode();
    });
  }
  
  void remove(Item* item);
  Item* find(const char* key, bool lru);
  Item* alloc(size_t data_size); 
  void add(Item* item);

private:
  HTable<const char*, Item*> hash_table_;
  ConcurrentSlabList& slablist_;
  LRUList lru_list_;
};

}
}
#endif