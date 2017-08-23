#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include <libnet/logger.h>

namespace libnet
{

EventLoopThread::EventLoopThread(const std::string& name)
  : name_(name),
    lock_(),
    cond_(lock_),
    loop_(NULL),
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
  loop_.load()->loop();
  loop_ = NULL;
};

EventLoop* EventLoopThread::getLoop()
{
  return loop_;
};


EventLoopThread::~EventLoopThread()
{
  LOG_TRACE << "~EventLoopThread()" ;
  EventLoop* loop = loop_.load();
  if (loop != NULL)
  {
    loop->shutdown();
    thread_.join();
  }
};

}