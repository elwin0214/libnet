#ifndef __LIBNET_MEMCACHED_SERVER_HTABLE_H__
#define __LIBNET_MEMCACHED_SERVER_HTABLE_H__

#include <stdint.h>
#include <sys/types.h>
#include <assert.h>
#include <list>
#include <vector>
#include <string>
#include <functional>

namespace mc
{
namespace server
{
using namespace std;
static const size_t kMaxHashPower = 30;

template<typename Key, typename Entry>
class HTable
{
public:
  typedef std::list<Entry> Bucket;

 HTable(size_t hashpower, double factor)
    : hashpower_(hashpower),
      size_(0),
      factor_(factor),
      new_hashpower_(0),
      moving_index_(-1),
      moving_(false),
      //hash_(std::hash<std::string>()),
      buckets_((1 << hashpower))
  {
  }

public:
  Entry get(Key key)
  {
    size_t hashcode = hash_(key);
    Bucket& bucket = getBucket(hashcode);
    typename Bucket::iterator itr = find(bucket, key);
    if (itr == bucket.end())
    {
      return NULL;
    } 
    return *itr;
  };

  Entry setItem(Entry entry)
  {
    Key key = get_key_(entry);
    size_t hashcode = get_hash_(entry);
    Bucket& bucket = getBucket(hashcode);
    typename Bucket::iterator itr = find(bucket, key);
    if (itr == bucket.end())
    {
      bucket.push_back(entry);
      size_++;
      resize();
      return NULL;  
    }
    Entry old = *itr; // get item before resize()
    bucket.erase(itr); 
    return old;
  };

  void addItem(Entry entry)
  {
    size_t hashcode = get_hash_(entry);
    Bucket& bucket = getBucket(hashcode);
    bucket.push_back(entry);
    size_++;
    resize();
  };

  Entry removeItem(Entry entry)
  {
    size_t hashcode = get_hash_(entry);
    Key key = get_key_(entry);
    Bucket& bucket = getBucket(hashcode);
    typename Bucket::iterator itr = find(bucket, key);
    if (itr == bucket.end()) return NULL;
    bucket.erase(itr); 
    size_--;
    return *itr;
  };

  Entry remove(Key key)
  {
    size_t hashcode = hash_(key);
    Bucket& bucket = getBucket(hashcode);
    typename Bucket::iterator itr = find(bucket, key);
    if (itr == bucket.end()) return NULL;
    bucket.erase(itr);  
    size_--;
    return *itr;
  };

  void resize()
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
      for (auto entry : moving_bucket)
      {
        //const char* key = item->key();
        size_t new_index = get_hash_(entry) & ((1 << new_hashpower_) - 1);
        new_buckets_[new_index].push_back(entry);
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

  void set_hash(std::function<size_t(Key key)> hash) { hash_ = hash; }
  void set_equals(std::function<bool(Key k1, Key k2)> equals) { equals_ = equals; }
  void set_getkey(std::function<Key(Entry)> getkey) { get_key_ = getkey; }
  void set_gethash(std::function<size_t(Entry)> gethash) { get_hash_ = gethash; }
  size_t size() { return size_; }
  bool moving(){ return moving_; }
  size_t buckets() { return buckets_.size();}

private:
  Bucket& getBucket(size_t hashcode)
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
  }

  typename Bucket::iterator find(Bucket& bucket, Key key)
  {
    if (bucket.empty()) 
      return bucket.end();
    for (auto itr = bucket.begin();
         itr != bucket.end();
         itr++)
    {
      if (equals_((*itr)->key(), key)) return itr;
    }
    return bucket.end();
  };

private:
  size_t hashpower_;
  size_t size_;
  double factor_;
  size_t new_hashpower_;
  ssize_t moving_index_; //moving bucket index
  bool moving_;
  std::vector<Bucket> buckets_;
  std::vector<Bucket> new_buckets_;

  std::function<size_t(Key key)> hash_;
  std::function<bool(Key k1, Key k2)> equals_;
  std::function<Key(Entry)> get_key_;
  std::function<size_t(Entry)> get_hash_;

};

}
}

#endif