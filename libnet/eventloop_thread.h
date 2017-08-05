#ifndef __LIBNET_EVENTLOOPTHREAD_H__
#define __LIBNET_EVENTLOOPTHREAD_H__

#include <string>
#include <atomic>
#include <libnet/mutexlock.h>
#include <libnet/condition.h>
#include <libnet/thread.h>

namespace libnet
{

class EventLoop;

class EventLoopThread : public NoCopyable 
{
public:
  EventLoopThread(const std::string& name);

  EventLoop* start();

  EventLoop* getLoop();

  ~EventLoopThread();
  
private:
  void exec();

private:
  std::string name_;
  MutexLock lock_;
  Condition cond_;
  std::atomic<EventLoop*> loop_;
  Thread thread_;
};

}

#endif