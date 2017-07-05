#ifndef __LIBNET_MEMCACHED_CLIENT_FUTURE_H__
#define __LIBNET_MEMCACHED_CLIENT_FUTURE_H__
#include <libnet/mutexlock.h>
#include <libnet/condition.h>
#include <libnet/nocopyable.h>

namespace memcached
{
namespace client
{

template<typename T>
class Future : public NoCopyable
{  
public:
  Future()
    : value_()
      lock_()
      condition_(lock_)
  {

  }

  T&& get()
  {
    LockGuard guard(lock_); 
    while(!inited_)
    {
      condition_.wait();
    }
    return std::move(value_);
  }

  void wait(int timeoutMs)
  {
    LockGuard guard(lock_); 
    if (inited_) return;
    if (timeoutMs <= 0) condition_.wait();
    else condition_.wait(timeoutMs);
  }

  void set_value(T&& value)
  {
    LockGuard guard(lock_); 
    value_ = std::move(value);
    inited_ = true;
    condition_.notifyAll();
  }

private:
  T value_;
  bool inited_;
  MutexLock lock_;
  Condition condition_;

};

}
}
#endif
