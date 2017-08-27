#ifndef __LIBNET_CONDITION_H__
#define __LIBNET_CONDITION_H__

#include <libnet/nocopyable.h>
#include <libnet/logger.h>
#include <libnet/mutexlock.h>
#include <pthread.h>
#include <libnet/timestamp.h>
#include <errno.h>

namespace libnet
{

class Condition : public NoCopyable
{
private:
  pthread_cond_t cond_;
  MutexLock &lock_;

public:
  Condition(MutexLock &lock):lock_(lock)
  {
    if (0 > pthread_cond_init(&cond_, NULL))
      LOG_SYSERROR << "thread = " << thread::currentTid() ;
  }

  void wait()
  {
    if (0 > pthread_cond_wait(&cond_, &(lock_.mutex)))
      LOG_SYSERROR << "thread = " << thread::currentTid() ;
  }

  int64_t wait(uint32_t wait_milli);

  //different with Lock & Object monitor in Java 
  //https://stackoverflow.com/questions/4544234/calling-pthread-cond-signal-without-locking-mutex
  void notifyAll()
  {
    if (0 > pthread_cond_broadcast(&cond_))
      LOG_SYSERROR << "thread = " << thread::currentTid() ;
  }

  void notify()
  {
    if (0 > pthread_cond_signal(&cond_))
      LOG_SYSERROR << "thread = " << thread::currentTid() ;
  }

  ~Condition()
  {
    if (0 > pthread_cond_destroy(&cond_))
      LOG_SYSERROR << "thread =" << thread::currentTid() ;
  }

};

}

#endif
