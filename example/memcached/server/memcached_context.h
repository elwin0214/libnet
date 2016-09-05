#ifndef __LIBNET_MEMCACHED_SERVER_CONTEXT_H__
#define __LIBNET_MEMCACHED_SERVER_CONTEXT_H__
#include <functional>
#include <libnet/nocopyable.h>
#include <string>

namespace memcached
{
namespace server
{

enum Opt
{
  kNo = -1,
  kAdd = 0,
  kReplace = 1,
  kSet = 2,
  kGet = 3,
  kDelete = 4,
  kIncr = 5,
  kDecr = 6
};

class MemcachedContext : public libnet::NoCopyable
{

public:
  MemcachedContext()
    : opt_(kNo),
      flags_(0),
      exptime_(0),
      bytes_(0),
      key_()
  {

  };

  void reset()
  {
    opt_ = kNo;
    flags_ = 0;
    exptime_ = 0;
    bytes_ = 0;
    std::string tmp;
    key_.swap(tmp);
  };

  void set_opt(Opt opt) { opt_ = opt; }
  Opt get_opt() { return opt_; }
  void set_flags(uint16_t flags) { flags_ = flags; }
  uint16_t get_flags() { return flags_; }

  void set_bytes(uint32_t bytes) { bytes_ = bytes; }
  uint32_t get_bytes() { return bytes_; }

  std::string& get_key(){ return key_; }
  void set_key(std::string&& key) { key_ = key; }

  void set_exptime(uint32_t exptime)
  {
    exptime_ =  exptime;
  }

  uint32_t get_exptime()
  {
    return exptime_;
  }

  void set_send_func(std::function<void(const char*)> func)
  {
    send_func_ = func;
  }

  void set_close_func(std::function<void()> func)
  {
    close_func_ = func;
  }

  void send(const char* str)
  {
    send_func_(str);
  }

  void close()
  {
    close_func_();
  }



private:
  Opt opt_;
  uint16_t flags_;
  uint32_t exptime_;
  uint32_t bytes_;
  std::string key_;
  std::function<void(const char*)> send_func_;
  std::function<void()> close_func_;

};


}
}

#endif