#ifndef __LIBNET_MEMCACHED_SERVER_ITEM_H__
#define __LIBNET_MEMCACHED_SERVER_ITEM_H__
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <functional>
#include <libnet/digits.h>
#include <libnet/nocopyable.h>

namespace memcached
{
namespace server
{
using namespace libnet;
class Item : public NoCopyable
{

public:
  Item(uint8_t index, uint32_t size)
    : index_(index),
      size_(size)
    {
      ::memset(data_, 0, size_);// the actual length of data is greater than size_
    }

  void clear()
  {
    prev_ = NULL;
    next_ = NULL;
    hash_ = 0;
    flags_ = 0;
    exptime_ = 0;
  }

  const char* key()
  {
    return data_;
  }

  char* value()
  {
    return data_ + key_size_ + 1 ;
  }

  void set_key(const char* key, uint8_t len)
  {
    ::memcpy(data_, key, len);
    data_[len] = '\0';
    key_size_ = len;
    hash_ = std::hash<std::string>()(std::string(data_, len));
  }

  void set_value(const char* value, uint16_t len)
  {
    assert(key_size_ >= 1);
    ::memcpy(data_ + key_size_ + 1, value, len);
    data_[key_size_ + len + 1] = '\0';
    bytes_ = len;
  }

  Item* prev() { return prev_; }
  Item* next() { return next_; }

  void setPrev(Item* item){ prev_ = item; }
  void setNext(Item* item){ next_ = item; }
  
  size_t index() {return index_; }
  size_t size() {return size_; }

  void set_flags(uint16_t flags) {flags_ = flags; }
  uint16_t get_flags() {return flags_; }
  
  uint32_t get_bytes() { return bytes_; }
  void set_exptime(uint32_t exptime) { exptime_ = exptime; }

private:
  ~Item(){} //can not be delete

public:
  Item* prev_;
  Item* next_;
  Item* hash_prev_;
  Item* hash_next_;

  const uint8_t index_;  //const
  const uint32_t size_; // const
  size_t hash_;
  uint16_t flags_; 
  uint32_t exptime_; 
  uint32_t bytes_;
  uint8_t key_size_;
  char data_[0]; //align

};
}
}

#endif