#include <vector>
#include "timer_queue.h"
#include "eventloop.h"

namespace libnet
{

TimerQueue::TimerQueue(EventLoop *loop):queue_(),loop_(loop)
{

};

TimerId TimerQueue::runAt(const Timestamp &timestamp, const Timer::TimerCallback &callback)
{
  Timer *timer = new Timer(timestamp, callback);
  loop_->queueInLoop(std::bind(&TimerQueue::runInLoop, this, timer));
  return TimerId(timer, timer->id());
};

TimerId TimerQueue::runAt(const Timestamp &timestamp, int intervalMs, const Timer::TimerCallback &callback)
{
  Timer *timer = new Timer(timestamp, intervalMs, callback);
  loop_->queueInLoop(std::bind(&TimerQueue::runInLoop, this, timer));
  return TimerId(timer, timer->id());
};

TimerId TimerQueue::runAfter(int timeMs, const Timer::TimerCallback &callback)
{
  Timestamp ts = Timestamp::now();
  ts.add(timeMs);
  return runAt(ts, callback);
};

TimerId TimerQueue::runAfter(int timeMs, int intervalMs, const Timer::TimerCallback &callback)
{
  Timestamp ts = Timestamp::now();
  ts.add(timeMs);
  return runAt(ts, intervalMs, callback);
};


void TimerQueue::cancel(TimerId timerId)
{
  loop_->assertInLoopThread();
  loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
};


void TimerQueue::runInLoop(Timer *timer)
{
  loop_->assertInLoopThread();
  LOG_TRACE << "queue.size=" << (queue_.size()) <<" add timer=" << (timer->id()) <<" time=" << (timer->time().value());
  queue_.insert(std::pair<const Timestamp, Timer*>(timer->time(), timer));

};

void TimerQueue::cancelInLoop(TimerId timerId)
{ 
  loop_->assertInLoopThread();
  cancelingTimers_.insert(timerId);
};

void TimerQueue::expire(const Timestamp &now)
{
  loop_->assertInLoopThread();
  LOG_TRACE << "begin queue.size=" << (queue_.size());
  if (queue_.size() == 0) return;
  Queue::iterator lmt = queue_.lower_bound(now);
  Queue::iterator itr = queue_.begin();
  std::vector<Timer*> expireTimers;
  expireTimers.reserve(100);
  for (; itr != lmt; )
  {

    (itr->second)->run(); //call 
    Queue::iterator tmp = itr;
    itr++;
    TimerId timerId(tmp->second, tmp->second->id());
    if (tmp->second->next() && cancelingTimers_.find(timerId) == cancelingTimers_.end())// wont be cancel
    {
        expireTimers.push_back(tmp->second);
    }
    else
    {
      LOG_TRACE << "remove timer = " << (tmp->second->id());
      delete tmp->second;
    }
    queue_.erase(tmp);
  }
  LOG_TRACE << "queue.size=" << (queue_.size());
  for (std::vector<Timer*>::iterator itr = expireTimers.begin(); itr != expireTimers.end(); itr++)
  {
    LOG_TRACE << "queue.size=" << (queue_.size()) <<" add timer=" << ((*itr)->id()) <<" time=" << (((*itr)->time()).value());
    queue_.insert(std::pair<const Timestamp, Timer*>((*itr)->time(), *itr));
  }
  cancelingTimers_.clear();
};

const Timestamp* TimerQueue::earliest()
{
  Queue::iterator itr = queue_.begin();
  if (itr == queue_.end()){
    LOG_TRACE << "size=" << queue_.size() << " earliest time is NULL.";
    return NULL;
  }
  LOG_TRACE << "size=" << queue_.size() << " earliest time is " << (itr->first).toString();
  return &(itr->first);
};

TimerQueue::~TimerQueue()
{
    //delete timer
  for (Queue::iterator itr = queue_.begin(); itr != queue_.end(); itr++)
  {
    delete itr->second;
  }
};

}
