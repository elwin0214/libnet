#ifndef __LIBNET_EVENTLOOPTHREAD_H__
#define __LIBNET_EVENTLOOPTHREAD_H__

#include "mutexlock.h"
#include "condition.h"
#include "thread.h"
#include <string>

namespace libnet
{

class EventLoop;

class EventLoopThread : public NoCopyable 
{
public:
  EventLoopThread(const std::string& name);

  EventLoop* start();

  EventLoop* getLoop();

  void exec();

  ~EventLoopThread();

private:
  std::string name_;
  MutexLock lock_;
  Condition cond_;
  EventLoop* loop_;
  Thread thread_;

};

}

#endif