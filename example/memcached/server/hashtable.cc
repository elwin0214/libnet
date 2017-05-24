#include "hashtable.h"

namespace memcached
{
namespace server
{

// class Bucket
// {
// public:
//   void add(Item* item)
//   {
//     if (NULL == head_)
//     {
//       head_ = item;
//       head_->hash_next_ = NULL;
//       head_->hash_prev_ = NULL;
//       return;
//     }

//     item->hash_next_ = head_;
//     item->hash_prev_ = NULL;
//     head_->hash_prev_ = item;
//     head_ = item;
//   }

//   Item* find(const char* key)
//   {
//     if (NULL ==  head_) return NULL;
//     Item* item = head_;
//     while (NULL != item)
//     {
//       if (::strcmp(item->key(), key) == 0)
//         return item;
//       item = item->hash_next_;
//     }
//     return NULL;
//   }

// private:
//   Item* head_;
//   size_t size_;

// };

static const size_t kMaxHashPower = 30;
// a hashtable moving one bucket every time when resizing
Item* HashTable::get(const char* key)
{
  size_t hashcode = hash(key);
  Bucket& bucket = getBucket(hashcode);
  Bucket::iterator itr = find(bucket, key);
  if (itr == bucket.end()) return NULL;
  return *itr;
};

Item* HashTable::setItem(Item* item)
{
  const char* key = item->key();
  size_t hashcode = item->hash_;
  Bucket& bucket = getBucket(hashcode);
  Bucket::iterator itr = find(bucket, key);
  if (itr == bucket.end())
  {
    bucket.push_back(item);
    size_++;
    resize();
    return NULL;  
  }
  Item* old = *itr; // get item before resize()
  bucket.erase(itr); 
  return old;
};

void HashTable::addItem(Item* item)
{
  size_t hashcode = item->hash_;
  Bucket& bucket = getBucket(hashcode);
  bucket.push_back(item);
  size_++;
  resize();
};

Item* HashTable::removeItem(Item* item)
{
  size_t hashcode = item->hash_;
  Bucket& bucket = getBucket(hashcode);
  Bucket::iterator itr = find(bucket, item->key());
  if (itr == bucket.end()) return NULL;
  bucket.erase(itr); 
  size_--;
  return *itr;
};

Item* HashTable::remove(const char* key)
{
  size_t hashcode = hash(key);
  Bucket& bucket = getBucket(hashcode);
  Bucket::iterator itr = find(bucket, key);
  if (itr == bucket.end()) return NULL;
  bucket.erase(itr);  
  size_--;
  return *itr;
};

HashTable::Bucket::iterator HashTable::find(Bucket& bucket, const char* key)
{
  if (bucket.empty()) 
    return bucket.end();
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

HashTable::Bucket& HashTable::getBucket(size_t hashcode)
{
  if (!moving_)
  {
    size_t index = (hashcode & ((1 << hashpower_) - 1));
    return buckets_[index];
  }
  else
  {
    size_t index = (hashcode & ((1 << hashpower_) - 1));
    if (index <= moving_index_)
    {
      size_t new_index = (hashcode & ((1 << new_hashpower_) - 1));
      return new_buckets_[new_index];
    }
    else
    {
      return buckets_[index];
    }
  }
};

void HashTable::resize()
{
  if ((!moving_) && (size() > buckets() * factor_) && hashpower_ < kMaxHashPower)
  {
    moving_ = true;
    new_hashpower_ = hashpower_ + 1;
    new_buckets_.resize(1 << new_hashpower_);
    moving_index_ = -1;
  }
  if (!moving_) return ;

  assert (moving_index_ < (1 << hashpower_) - 1);
  moving_index_++;

  Bucket& moving_bucket = buckets_[moving_index_];
  if (!moving_bucket.empty())
  {
    for (auto item : moving_bucket)
    {
      //const char* key = item->key();
      size_t new_index = item->hash_ & ((1 << new_hashpower_) - 1);
      new_buckets_[new_index].push_back(item);
    }
    moving_bucket.clear();
  }

  if (moving_index_ == (1 << hashpower_) - 1)
  {
    moving_ = false; //finished
    moving_index_ = -1;
    hashpower_ = new_hashpower_;
    buckets_ = std::move(new_buckets_);
    new_buckets_.clear();
  }
  
}


}
}