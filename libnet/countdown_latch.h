#ifndef __LIBNET_COUNTDOWNLATCH_H__
#define __LIBNET_COUNTDOWNLATCH_H__

#include <libnet/mutexlock.h>
#include <libnet/condition.h>

namespace libnet
{

class CountDownLatch : public NoCopyable
{
public:
  explicit CountDownLatch(int count)
    : lock_(),
      condition_(lock_),
      count_(count)
  {

  };

  void wait()
  {
    LockGuard guard(lock_);
    while (count_ > 0)
    {
      condition_.wait();
    }
  };

  void countDown()
  {
    LockGuard guard(lock_);
    count_--;
    if (count_ == 0)
      condition_.notifyAll();
  };

  int count()
  {
    LockGuard guard(lock_);
    return count_;
  };

private:
  MutexLock lock_;
  Condition condition_;
  int count_;
};

}
#endif