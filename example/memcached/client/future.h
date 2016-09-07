#ifndef __LIBNET_MEMCACHED_CLIENT_FUTURE_H__
#define __LIBNET_MEMCACHED_CLIENT_FUTURE_H__
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include "code.h"

namespace memcached
{
namespace client
{

template<typename T>
class Future : public NoCopyable
{  
public:
  Future()
    : code_(kInit),
      latch_(1)
  {

  }

  void wait() { latch_.wait(); }
  void wakeup() {latch_.countDown(); }
   
  void set(Code code, const T& result)
  {
    code_ = code;
    result_ = result;
  }

  Code code() { return code_; }
  T result() { return result_; }

private:
  Code code_;
  T result_;
  CountDownLatch latch_;

};

}
}
#endif
