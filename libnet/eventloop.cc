#include "eventloop.h"
#include "channel.h"
#include "logger.h"
#include "selector/selector.h"
#include "selector/selector_provider.h"
#include "current_thread.h"
#include "socket_ops.h"
#include "timer_queue.h"
#include "timestamp.h"
#include "exception.h"
#include <assert.h>

namespace libnet
{
namespace loop
{
const int kSelectTimeMs = 10000;
}

EventLoop::EventLoop()
  : selector_(selector::SelectorProvider::provide(this)),
    functors_(),
    tid_(thread::currentTid()),
    stop_(false),
    wakeup_(false),
    timerQueue_(new TimerQueue(this))
{
  LOG_TRACE << "EventLoop.tid = " << tid_ << " current.tid = "<< thread::currentTid();
  sockets::createPipe(wakeupFd_);
  LOG_TRACE << "create pipe=" << (wakeupFd_[0]) << " and " << wakeupFd_[1] ;
  wakeupChannel_ = new Channel(this, wakeupFd_[0]);

  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
};

EventLoop::~EventLoop()
{
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  delete wakeupChannel_;
  sockets::close(wakeupFd_[0]); 
  sockets::close(wakeupFd_[1]); 
};

void EventLoop::handleRead()
{   
  char buf[128];
  ssize_t n = sockets::read(wakeupFd_[0], buf, 128);
  if (n < 0)
    LOG_ERROR << "read fd-" << wakeupFd_[0] << " ,size-" <<n << ", errno-" << errno ;
};

void EventLoop::loop()
{
  assertInLoopThread();
  while(!stop_)//stop = true 如果queue 还有任务？
  {

    const Timestamp* earliestTimeStamp = timerQueue_->earliest();
    std::vector<Channel*> activeChannles;
    wakeup_.set(false);
    if (NULL == earliestTimeStamp)
      selector_->select(loop::kSelectTimeMs, activeChannles);
    else
    {
      Timestamp now = Timestamp::now();
      int timeoutMs = ((earliestTimeStamp->value() - now.value()) / 1000);
      timeoutMs = timeoutMs < loop::kSelectTimeMs ? timeoutMs : loop::kSelectTimeMs;
      LOG_DEBUG << "loop now-" <<  (now.value()) << " ,early-" << (earliestTimeStamp->value()) << " ,timeout-" << timeoutMs;
      selector_->select(timeoutMs, activeChannles);
    }
        //if (activeChannles.size() <= 0) continue;

    if (activeChannles.size() > 0)
    {
      for (std::vector<Channel*>::iterator itr = activeChannles.begin(); itr != activeChannles.end(); itr++)
      {
        (*itr)->handleEvent();
      }
    }
    {//functor 要放在后面，loop停止的时候 仍然会执行queue中的 functor
      LockGuard guard(lock_);
      LOG_DEBUG << "functors.size=" << functors_.size();
      while (!functors_.empty())
      {
        Functor& func = functors_.front();
        func();
        functors_.pop();
      }
    }
    if (!stop_)
    {
      Timestamp now = Timestamp::now();
      timerQueue_->expire(now);
    }
  }
};

TimerId EventLoop::runAt(const Timestamp &timestamp, const Functor& functor)
{
  return timerQueue_->runAt(timestamp, functor);
};

TimerId EventLoop::runAfter(int afterTimeMs, const Functor& functor)
{
  Timestamp timestamp = Timestamp::now();
  timestamp.add(afterTimeMs);
  return timerQueue_->runAt(timestamp, functor);
};

TimerId EventLoop::runInterval(int afterTimeMs, int intervalMs, const Functor& functor)
{
  Timestamp timestamp = Timestamp::now();
  timestamp.add(afterTimeMs);
  LOG_TRACE << "runInterval timestamp at " <<timestamp.value();
  return timerQueue_->runAt(timestamp, intervalMs, functor);
};

TimerId EventLoop::runAt(const Timestamp &timestamp, Functor&& functor)
{
  return timerQueue_->runAt(timestamp, std::move(functor));
};

TimerId EventLoop::runAfter(int afterTimeMs, Functor&& functor)
{
  Timestamp timestamp = Timestamp::now();
  timestamp.add(afterTimeMs);
  return timerQueue_->runAt(timestamp, std::move(functor));
};

TimerId EventLoop::runInterval(int afterTimeMs, int intervalMs, Functor&& functor)
{
  Timestamp timestamp = Timestamp::now();
  timestamp.add(afterTimeMs);
  LOG_TRACE << "runInterval timestamp at " <<timestamp.value();
  return timerQueue_->runAt(timestamp, intervalMs, std::move(functor));
};

void EventLoop::runInLoop(const Functor& functor)
{
  if (inLoopThread())
  {
    functor();
  }
  else
  {
    queueInLoop(functor);
  }
};

void EventLoop::queueInLoop(const Functor& functor)
{  
  {
    LockGuard guard(lock_);
    functors_.push(functor);
  }
  if (!inLoopThread())
    wakeup();
};

void EventLoop::runInLoop(Functor&& functor)
{
  if (inLoopThread())
  {
    functor();
  }
  else
  {
    queueInLoop(std::move(functor));
  }
};

void EventLoop::queueInLoop(Functor&& functor)
{  
  {
    LockGuard guard(lock_);
    functors_.push(std::move(functor));
  }
  if (!inLoopThread())
    wakeup();
};


bool EventLoop::inLoopThread()
{
  return tid_ == thread::currentTid();
};

void EventLoop::assertInLoopThread()
{
  if (!inLoopThread()){
    LOG_SYSFATAL << "current thread ID "<< thread::currentTid() << ", but the EventLoop thread ID " << tid_;
  }

};

void EventLoop::updateChannel(Channel* channel)
{
  assertInLoopThread();
  selector_->updateChannel(channel);
};

void EventLoop::removeChannel(Channel* channel)
{
  assertInLoopThread();
  selector_->removeChannel(channel);
};

void EventLoop::shutdown()
{
  stop_ = true;
};

void EventLoop::wakeup()
{
  //if (!wakeup_.cas(false, true))
  //{
  //  LOG_TRACE << "cas fail! fd=" << (wakeupFd_[1]);
  //  return;
  //}
  LOG_TRACE << "goto write fd=" << (wakeupFd_[1]);
  ssize_t n = sockets::write(wakeupFd_[1], "a", 1);
  if (n < 0)
    LOG_SYSERROR << "wake up, fd=" << (wakeupFd_[1]);
};

}