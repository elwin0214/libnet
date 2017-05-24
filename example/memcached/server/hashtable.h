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
#include <libnet/nocopyable.h>
#include "item.h"

namespace memcached
{
namespace server
{

class HashTable : public libnet::NoCopyable
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
      new_hashpower_(0),
      moving_index_(-1),
      moving_(false),
      hash_(std::hash<std::string>()),
      buckets_(1 << hashpower)
  {

  }

  size_t hash(const char* key)
  {
    return hash_(key);
  }

  Item* setItem(Item* item);

  void addItem(Item* item);

  Item* removeItem(Item* item);

  Item* get(const char* key);

  Item* remove(const char* key);

  size_t size() { return size_; }

  size_t buckets() { return buckets_.size();}

  bool moving(){ return moving_; }

private:

  void add(size_t index, Item* item);

  void resize();

  HashTable::Bucket::iterator find(Bucket& bucket, const char* key);

  HashTable::Bucket& getBucket(size_t hashcode);


private:

  size_t hashpower_;
  size_t size_;
  double factor_;
  size_t new_hashpower_;
  ssize_t moving_index_; //moving bucket index
  bool moving_;
  std::function<size_t(const std::string&)> hash_;
  std::vector<Bucket> buckets_;
  std::vector<Bucket> new_buckets_;

};

}
}

#endif