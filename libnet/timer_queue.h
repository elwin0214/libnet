#ifndef __LIBNET_TIMERQUEUE_H__
#define __LIBNET_TIMERQUEUE_H__
#include <functional>
#include <map>
#include <set>
#include <libnet/nocopyable.h>
#include <libnet/timer.h>
#include <libnet/timestamp.h>

namespace libnet
{

class EventLoop;
class Timer;

class TimerQueue: public NoCopyable
{

public:

  typedef std::pair<Timestamp, Timer*> Entry;
  typedef std::set<Entry> TimerList;
  typedef std::pair<Timer*, uint64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerList;

  TimerQueue(EventLoop *loop);

  ~TimerQueue();

  // run at given time
  TimerId runAt(const Timestamp &timestamp, const Timer::TimerCallback& callback);

  TimerId runAt(const Timestamp &timestamp, int interval, const Timer::TimerCallback& callback);

  TimerId runAt(const Timestamp &timestamp, Timer::TimerCallback&& callback);

  TimerId runAt(const Timestamp &timestamp, int interval, Timer::TimerCallback&& callback);

  TimerId runAfter(int delay, const Timer::TimerCallback &callback)
  {
    Timestamp ts = Timestamp::now();
    ts.add(delay);
    return runAt(ts, callback);
  }

  TimerId runAfter(int delay, int interval, const Timer::TimerCallback& callback)
  {
    Timestamp ts = Timestamp::now();
    ts.add(delay);
    return runAt(ts, interval, callback);
  }

  TimerId runAfter(int delay, Timer::TimerCallback&& callback)
  {
    Timestamp ts = Timestamp::now();
    ts.add(delay);
    return runAt(ts, std::move(callback));
  }

  TimerId runAfter(int delay, int interval, Timer::TimerCallback&& callback)
  {
    Timestamp ts = Timestamp::now();
    ts.add(delay);
    return runAt(ts, interval, std::move(callback));
  }

  void cancel(TimerId timer_id);

  // run all the callback before the given time
  void expire(const Timestamp &timestamp);

  const Timestamp* earliest();

  size_t size() {return timers_.size(); } 

private:
  void addInLoop(Timer *timer);
  void cancelInLoop(TimerId timerId);

private:
  EventLoop *loop_;
  TimerList timers_;
  ActiveTimerList canceling_timers_;
};

}
#endif

