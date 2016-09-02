#ifndef __LIBNET_MEMCACHED_SERVER_ITEM_H__
#define __LIBNET_MEMCACHED_SERVER_ITEM_H__
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <libnet/digits.h>

namespace memcached
{
namespace server
{

class Item
{
public:

  void reset(uint16_t size)
  {
    size_ = size;
    ::memset(data_, 0, size_);
  }

  void clear()
  {
    prev_ = NULL;
    next_ = NULL;
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


  // uint32_t expireTime()
  // {
  //   return stringToDigit
  // } 

  void set_key(const char* key, uint8_t len)
  {
    ::memcpy(data_, key, len);
    data_[len] = '\0';
    key_size_ = len;
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

public:
  Item* prev_;
  Item* next_;
  //Item* hash_prev_;
  //Item* hash_next_;

  uint8_t index_;  //const
  uint32_t size_; // const
  uint16_t flags_; 
  uint32_t exptime_; 
  uint32_t bytes_;
  uint8_t key_size_;
  char data_[0]; //align

};
//VALUE <key> <flags> <bytes> [<cas unique>]\r\n<data block>\r\nEND\r\n  
// key value expires 
}
}

#endif