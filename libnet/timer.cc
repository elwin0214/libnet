#include "timer.h"

namespace libnet
{


AtomicInt64 Timer::s_timerId_(0);

Timer::Timer(Timestamp time, int interval, const TimerCallback& callback)
  : time_(time),
    interval_(interval),
    callback_(callback),
    id_(s_timerId_.getAndAdd(1))
{
};

Timer::Timer(Timestamp time, int interval, TimerCallback&& callback)
  : time_(time),
    interval_(interval),
    callback_(std::move(callback)),
    id_(s_timerId_.getAndAdd(1))
{
};

Timer::Timer(Timestamp time, const TimerCallback& callback)
  : time_(time),
    interval_(0),
    callback_(callback),
    id_(s_timerId_.getAndAdd(1))
{
};

Timer::Timer(Timestamp time, TimerCallback&& callback)
  : time_(time),
    interval_(0),
    callback_(std::move(callback)),
    id_(s_timerId_.getAndAdd(1))
{
};

bool Timer::next()
{
  if (interval_ <= 0)
    return false;
  time_.add(interval_);
  return true;
};

}