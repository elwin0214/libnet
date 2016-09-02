#ifndef __LIBNET_MEMCACHED_SERVER_HASHTABLE_H__
#define __LIBNET_MEMCACHED_SERVER_HASHTABLE_H__
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <functional>
#include <vector>
#include <list>
#include <string>
#include <assert.h>
#include "item.h"

namespace memcached
{
namespace server
{

class HashTable
{
public:
  typedef Item* ItemPtr;
  typedef std::function<void(ItemPtr)> Callback;
  typedef std::list<ItemPtr> Bucket;

public:
  HashTable(size_t hashpower, double factor)
    : hashpower_(hashpower),
      size_(0),
      factor_(factor),
      moving_(false),
      hash_(std::hash<std::string>()),
      buckets_(1 << hashpower)
  {

  }

  size_t hash(const char* key)
  {
    return hash_(key);
  }

  Item* get(const char* key);

  Item* set(Item* item);

  Item* remove(const char* key);

  size_t size(){return size_; }

  size_t buckets() {return buckets_.size();}

  bool moving(){ return moving_; }

  bool resize(){return true; };


private:
  void add(size_t index, Item* item);

  HashTable::Bucket::iterator find(size_t bucket_index, const char* key);


private:

  size_t hashpower_;
  size_t size_;
  double factor_;
  size_t moving_bucket_;
  bool moving_;
  std::function<size_t(const std::string&)> hash_;
  std::vector<Bucket> buckets_;
  std::vector<Bucket> moving_buckets_;

};

}
}

#endif