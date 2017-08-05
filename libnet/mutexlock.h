#ifndef __LIBNET_THREAD_MUTEXLOCK_H__
#define __LIBNET_THREAD_MUTEXLOCK_H__

#include <errno.h>
#include <libnet/nocopyable.h>
#include <libnet/current_thread.h>
#include <libnet/logger.h>

namespace libnet
{

class MutexLock : public NoCopyable
{
friend class Condition;

private:
  pthread_mutex_t mutex;

public:
  MutexLock()
  {
    if (0 > pthread_mutex_init(&mutex, NULL))
      LOG_SYSERROR << "tid = " << thread::currentTid();
  }
  
  ~MutexLock()
  {
    if (0 > pthread_mutex_destroy(&mutex))
      LOG_SYSERROR << "tid = " << thread::currentTid();
  }

  void lock()
  {
    if (0 > pthread_mutex_lock(&mutex))
      LOG_SYSERROR << "tid = " << thread::currentTid();
  }
    
  void unlock()
  {
    if (0 > pthread_mutex_unlock(&mutex))
      LOG_SYSERROR << "tid = " << thread::currentTid();
  }
};


class LockGuard : public NoCopyable
{
private:
  MutexLock &lock_;
public:
  LockGuard(MutexLock &lock):lock_(lock)
  {
    LOG_TRACE << "LockGuard" ;
    lock_.lock();
  }
  
  ~LockGuard()
  {
    LOG_TRACE << "~LockGuard" ;
    lock_.unlock();
  }

};

}

#endif