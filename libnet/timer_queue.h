#ifndef __LIBNET_TIMERQUEUE_H__
#define __LIBNET_TIMERQUEUE_H__
#include <functional>
#include <map>
#include <set>
#include "nocopyable.h"
#include "timer.h"

namespace libnet
{

class EventLoop;
class Timer;


class TimerQueue: public NoCopyable
{

public:

  typedef std::multimap<Timestamp, Timer*> Queue;

  TimerQueue(EventLoop *loop);

  ~TimerQueue();

  // run at given time
  TimerId runAt(const Timestamp &timestamp, const Timer::TimerCallback &callback);

  TimerId runAt(const Timestamp &timestamp, int intervalMs, const Timer::TimerCallback &callback);

  TimerId runAfter(int timeMs, const Timer::TimerCallback &callback);

  TimerId runAfter(int timeMs, int intervalMs, const Timer::TimerCallback &callback);

  void cancel(TimerId timerId);

  // run all the callback before the given time
  void expire(const Timestamp &timestamp);

  const Timestamp* earliest();

  size_t size() {return queue_.size(); } 

private:
  void runInLoop(Timer *timer);
  void cancelInLoop(TimerId timerId);

private:
    Queue queue_;
    EventLoop *loop_;
    std::set<TimerId> cancelingTimers_;
};

}
#endif

