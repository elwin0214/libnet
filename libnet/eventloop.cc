#include <stdio.h>
#include <libnet/eventloop.h>
#include <libnet/channel.h>
#include <libnet/logger.h>
#include "selector/selector.h"
#include "selector/selector_provider.h"
#include <libnet/current_thread.h>
#include <libnet/socket_ops.h>
#include <libnet/timer_queue.h>
#include <libnet/timestamp.h>
#include <libnet/exception.h>
#include <assert.h>
#include <signal.h>

namespace libnet
{
namespace loop
{
const int kSelectTimeMs = 10000;
}
namespace signal
{
struct IgnoreSigPipe
{
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN); 
    LOG_TRACE << " errno = " << errno;
  }
};
}

signal::IgnoreSigPipe gIgnoreSigPipe;

EventLoop::EventLoop()
  : selector_(selector::SelectorProvider::provide(this)),
    functors_(),
    tid_(thread::currentTid()),
    stop_(false),
    exist_loopadded_functor_(false),
    //wakeup_(false),
    //existPendingFunctor_(false),
    timer_queue_(new TimerQueue(this))
{
  LOG_TRACE << "EventLoop.tid = " << tid_ << " current.tid = "<< thread::currentTid();
  sockets::createPipe(wakeupFd_);
  LOG_TRACE << "create pipe = " << (wakeupFd_[0]) << " and " << wakeupFd_[1] ;
  wakeupChannel_ = new Channel(this, wakeupFd_[0]);

  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
};

EventLoop::~EventLoop()
{
  LOG_TRACE << "~EventLoop()" ;
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
  LOG_TRACE << "read fd = " <<  wakeupFd_[0] << ", size = " << n;
  if (n < 0)
    LOG_ERROR << "read fd = " << wakeupFd_[0] << " ,size = " <<n << ", errno = " << errno ;
};

void EventLoop::loop()
{
  assertInLoopThread();
  while(!stop_)//stop = true 如果queue 还有任务？
  {
    std::vector<Channel*> active_channles;
    //wakeup_.set(false);
    // place 1
    if (exist_loopadded_functor_)
    {
      selector_->select(0, active_channles);
    }
    else
    {
      const Timestamp* earliest_ts = timer_queue_->earliest();
      if (NULL == earliest_ts)
        selector_->select(loop::kSelectTimeMs, active_channles);
      else
      {
        Timestamp now = Timestamp::now();
        int timeout_ms = ((earliest_ts->value() - now.value()) / 1000);
        timeout_ms = timeout_ms < loop::kSelectTimeMs ? timeout_ms : loop::kSelectTimeMs;
        timeout_ms = timeout_ms < 0 ? 0 : timeout_ms;
        LOG_TRACE << "loop now = " <<  (now.value()) << " ,early = " << (earliest_ts->value()) << " ,timeout = " << timeout_ms;
        selector_->select(timeout_ms, active_channles);
      }
    }


    if (active_channles.size() > 0)
    {
      for (std::vector<Channel*>::iterator itr = active_channles.begin(); itr != active_channles.end(); itr++)
      {
        (*itr)->handleEvent();
      }
    }
    std::queue<Functor>  functors;
    {//functor 要放在后面，loop停止的时候 仍然会执行queue中的 functor
      LockGuard guard(lock_);
      LOG_DEBUG << "functors.size = " << functors_.size();
      functors.swap(functors_);

    }
    // it maybe dont work, if we add a wakeup variable like Reactor.java in xmemcached-client.
    // if one "add(functor)" was called when Loop running in place 1, and when Loop run here, 
    // wakeup will be ture, if the add(functor) is called again at this time and will not be called
    // after setting wakeup to false, the functor added later will run after selectTimeMs.  
    //callingPendingFunctor_ = true;  // some functor may be add by func() in the while.
    exist_loopadded_functor_ = false;
    while (!functors.empty())
    {
      Functor& func = functors.front();
      try
      {
        func();
      }
      catch(const std::exception& e)
      {
        LOG_ERROR << " exception = " << e.what();
        fprintf(stderr, "exception = %s\n", e.what());
        throw Exception(e.what());
      }  
      functors.pop();
    }
    //callingPendingFunctor_ = false;

    if (!stop_)
    {
      Timestamp now = Timestamp::now();
      timer_queue_->expire(now);
    }
  }
};

TimerId EventLoop::runAt(const Timestamp &timestamp, const Functor& functor)
{
  return timer_queue_->runAt(timestamp, functor);
};

TimerId EventLoop::runAfter(int delay, const Functor& functor)
{
  
  Timestamp timestamp = Timestamp::now();
  timestamp.add(delay);
  return timer_queue_->runAt(timestamp, functor);
};

TimerId EventLoop::runInterval(int delay, int interval, const Functor& functor)
{
  Timestamp timestamp = Timestamp::now();
  timestamp.add(delay);
  LOG_TRACE << "runInterval timestamp at " <<timestamp.value();
  return timer_queue_->runAt(timestamp, interval, functor);
};

TimerId EventLoop::runAt(const Timestamp &timestamp, Functor&& functor)
{
  return timer_queue_->runAt(timestamp, std::move(functor));
};

TimerId EventLoop::runAfter(int delay, Functor&& functor)
{
  Timestamp timestamp = Timestamp::now();
  timestamp.add(delay);
  return timer_queue_->runAt(timestamp, std::move(functor));
};

TimerId EventLoop::runInterval(int delay, int interval, Functor&& functor)
{
  Timestamp timestamp = Timestamp::now();
  timestamp.add(delay);
  LOG_TRACE << "runInterval timestamp at " <<timestamp.value();
  return timer_queue_->runAt(timestamp, interval, std::move(functor));
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
  LOG_TRACE << " queueInLoop " 
          << " tid = "
          << tid_;
  {
    LockGuard guard(lock_);
    functors_.push(functor);
  }
  if (!inLoopThread()) wakeup();
  else exist_loopadded_functor_ = true;//loop thread
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
  LOG_TRACE << " queueInLoop " 
            << " tid = "
            << tid_;
  {
    LockGuard guard(lock_);
    functors_.push(std::move(functor));
  }
  if (!inLoopThread()) wakeup();
  else exist_loopadded_functor_ = true; // loop thread
};


bool EventLoop::inLoopThread()
{
  return tid_ == thread::currentTid();
};

void EventLoop::assertInLoopThread()
{
  if (!inLoopThread()){
    LOG_SYSFATAL << "error = thread dont match, current = "
                 << thread::currentTid() 
                 << " loop =  " 
                 << tid_ << " "
                 << Exception("dont match").stackTrace();
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
  queueInLoop(std::bind(&EventLoop::shutdownInLoop, this));
};

void EventLoop::shutdownInLoop()
{
  assertInLoopThread();
  stop_ = true;
}

void EventLoop::wakeup()
{
  //if (!wakeup_.cas(false, true))
  //{
  //  LOG_TRACE << "cas fail! fd=" << (wakeupFd_[1]);
  //  return;
  //}
  
  // if the loop thread write the wakeupFd
  // the dead lock maybe appear in this place between outter thread and loop thread
  ssize_t n = sockets::write(wakeupFd_[1], "a", 1);  
  LOG_TRACE << "write fd = " << (wakeupFd_[1]) << " size = " << n;
  if (n < 0)
    LOG_SYSERROR << "wake up, fd = " << (wakeupFd_[1]);
};

}