#include "hashtable.h"

namespace memcached
{
namespace server
{

Item* HashTable::get(const char* key)
{
  size_t hashcode = hash(key);
  size_t bucket_index = hashcode & ((1 << hashpower_) - 1 );
  Bucket::iterator itr = find(bucket_index, key);
  if (itr == buckets_[bucket_index].end()) return NULL;
  return *itr;
};

Item* HashTable::set(Item* item)
{
  const char* key = item->key();
  size_t hashcode = hash(key);
  size_t bucket_index = hashcode & ((1 << hashpower_) - 1);
  Bucket::iterator itr = find(bucket_index, key);
  buckets_[bucket_index].erase(itr); //check end itr?
  buckets_[bucket_index].push_back(item);
  if (itr == buckets_[bucket_index].end())
  {
    size_++;
    return NULL;  // nullptr == NULL ??
  }
  return *itr;
};

Item* HashTable::remove(const char* key)
{
  size_t hashcode = hash(key);
  size_t bucket_index = hashcode & ((1 << hashpower_) - 1);
  Bucket::iterator itr = find(bucket_index, key);
  buckets_[bucket_index].erase(itr); //check end itr?
  if (itr == buckets_[bucket_index].end()) return NULL;
  size_--;
  return *itr;
};

HashTable::Bucket::iterator HashTable::find(size_t bucket_index, const char* key)
{
  if (buckets_[bucket_index].empty()) 
    return buckets_[bucket_index].end();
  Bucket& bucket = buckets_[bucket_index];

  for (std::list<ItemPtr>::iterator itr = bucket.begin();
      itr != bucket.end();
      itr++)
  {
    if (::strcmp((*itr)->key(), key) == 0)
    {
      return itr;
    }
  }
  return bucket.end();
};

}
}