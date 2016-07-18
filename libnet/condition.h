#ifndef __LIBNET_CONDITION_H__
#define __LIBNET_CONDITION_H__

#include "nocopyable.h"
#include "logger.h"
#include "mutexlock.h"
#include <pthread.h>

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
    if (0 < pthread_cond_init(&cond_, NULL))
      LOG_ERROR << "thread -" << thread::currentTid() <<",errno-" << errno;
  }

  void wait()
  {
    if (0 < pthread_cond_wait(&cond_,  &(lock_.mutex)))
      LOG_ERROR << "thread -" << thread::currentTid() <<",errno-" << errno;
  }

  void notifyAll()
  {
    if (0 < pthread_cond_broadcast(&cond_))
      LOG_ERROR << "thread -" << thread::currentTid() <<",errno-" << errno;
  }

  ~Condition()
  {
    if (0 < pthread_cond_destroy(&cond_))
      LOG_ERROR << "thread -" << thread::currentTid() <<",errno-" << errno;
  }

};

}

#endif
