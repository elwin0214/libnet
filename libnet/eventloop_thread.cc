#include "eventloop.h"
#include "eventloop_thread.h"
#include "thread.h"

namespace libnet
{

EventLoopThread::EventLoopThread(const std::string& name)
  : name_(name),
    lock_(),
    cond_(lock_),
    thread_(std::bind(&EventLoopThread::exec, this), name_)
{

};

EventLoop* EventLoopThread::start()
{
  thread_.start();
  {
    LockGuard guard(lock_);
    while (!loop_)
    {
      cond_.wait();
    }
  }
  return loop_;
};

void EventLoopThread::exec()
{
  EventLoop loop;
  {
    LockGuard guard(lock_);
    loop_ = &loop;
    cond_.notifyAll();
  }
  loop_->loop();
  loop_ = NULL;
};

EventLoop* EventLoopThread::getLoop()
{
  return loop_;
};


EventLoopThread::~EventLoopThread()
{
  if (loop_)
  {
    loop_->shutdown();
    thread_.join();
  }
};

}