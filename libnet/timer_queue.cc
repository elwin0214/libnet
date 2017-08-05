#include <assert.h>
#include <vector>
#include <libnet/timer_queue.h>
#include <libnet/eventloop.h>

namespace libnet
{

TimerQueue::TimerQueue(EventLoop *loop)
  : loop_(loop),
    timers_(),
    canceling_timers_()
{

};

TimerId TimerQueue::runAt(const Timestamp& timestamp, const Timer::TimerCallback& callback)
{
  Timer *timer = new Timer(timestamp, callback);
  loop_->runInLoop(std::bind(&TimerQueue::addInLoop, this, timer));
  return TimerId(timer, timer->id());
};

TimerId TimerQueue::runAt(const Timestamp& timestamp, int interval, const Timer::TimerCallback& callback)
{
  Timer *timer = new Timer(timestamp, interval, callback);
  loop_->runInLoop(std::bind(&TimerQueue::addInLoop, this, timer));
  return TimerId(timer, timer->id());
};

TimerId TimerQueue::runAt(const Timestamp& timestamp, Timer::TimerCallback&& callback)
{
  Timer *timer = new Timer(timestamp, std::move(callback));
  loop_->runInLoop(std::bind(&TimerQueue::addInLoop, this, timer));
  return TimerId(timer, timer->id());
};

TimerId TimerQueue::runAt(const Timestamp& timestamp, int interval, Timer::TimerCallback&& callback)
{
  Timer *timer = new Timer(timestamp, interval, std::move(callback));
  loop_->runInLoop(std::bind(&TimerQueue::addInLoop, this, timer));
  return TimerId(timer, timer->id());
};


void TimerQueue::cancel(TimerId timer_id)
{
  loop_->assertInLoopThread();
  loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timer_id));
};


void TimerQueue::addInLoop(Timer *timer)
{
  loop_->assertInLoopThread();
  LOG_TRACE <<" add timer = " << (timer->id()) <<" time = " << (timer->time().value());
  timers_.insert(Entry(timer->time(), timer));

};

void TimerQueue::cancelInLoop(TimerId timer_id)
{ 
  loop_->assertInLoopThread();
  Timer* timer = timer_id.timer();
  auto itr = timers_.find(Entry(timer->time(), timer));
  if (itr != timers_.end())
  {
    timers_.erase(itr);
  }
  else
  {
    canceling_timers_.insert(ActiveTimer(timer, timer->id()));
  }
};

void TimerQueue::expire(const Timestamp &now)
{
  loop_->assertInLoopThread();
  if (timers_.size() == 0) 
  { 
    LOG_TRACE << "size is null";
    return;
  }
  TimerList::iterator end = timers_.lower_bound(Entry(now, reinterpret_cast<Timer*>(UINTPTR_MAX)));
  assert(end == timers_.end() || now < end->first);
  std::vector<Timer*> active_timers;
  std::vector<Timer*> next_timers;
  active_timers.reserve(128);
  next_timers.reserve(128);
  for (auto itr = timers_.begin(); itr != end; itr++)
  {
    active_timers.push_back(itr->second);
  }
  timers_.erase(timers_.begin(), end);

  for (auto itr = active_timers.begin(); itr != active_timers.end(); itr++)
  {
    Timer* timer = *itr;
    if (canceling_timers_.find(ActiveTimer(timer, timer->id())) != canceling_timers_.end())
    {
      LOG_TRACE << "timer " << timer->id() << " delete for cancel.";
      delete timer;
      continue;
    }
    LOG_TRACE << "timer " << timer->id() << " run.";
    timer->run();
    if (!timer->next())
    {
      LOG_TRACE << "timer " << timer->id() << " delete for finished.";
      delete timer;
      continue;
    }
    next_timers.push_back(timer);
  }
  for (auto itr = next_timers.begin(); itr != next_timers.end(); itr++)
  {
    LOG_TRACE <<" add timer = " << ((*itr)->id()) <<" time = " << (((*itr)->time()).value());
    timers_.insert(Entry((*itr)->time(), *itr));
  }
  canceling_timers_.clear();
};

const Timestamp* TimerQueue::earliest()
{
  auto itr = timers_.begin();
  if (itr == timers_.end()){
    LOG_TRACE << " earliest time is NULL.";
    return NULL;
  }
  LOG_TRACE << " earliest time is " << (itr->first).toString();
  return &(itr->first);
};

TimerQueue::~TimerQueue()
{
    //delete timer
  for (auto itr = timers_.begin(); itr != timers_.end(); itr++)
  {
    delete itr->second;
  }
};

}
