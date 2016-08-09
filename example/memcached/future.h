#ifndef __LIBNET_MEMCACHED_FUTURE_H__
#define __LIBNET_MEMCACHED_FUTURE_H__
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include "code.h"

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
   
  void set(Code code, const std::string& result)
  {
    code_ = code;
    result_ = result;
  }

  Code code() { return code_; }
  std::string result() { return result_; }

private:
  Code code_;
  std::string result_;
  CountDownLatch latch_;

};
#endif
