#include "timer.h"

namespace libnet
{


AtomicInt64 Timer::s_timerId_(0);

Timer::Timer(Timestamp time, int inervalMs, const TimerCallback &timerCallback)
  : time_(time),
    intervalMs_(inervalMs),
    callback_(timerCallback),
    id_(s_timerId_.getAndAdd(1))
{
};

Timer::Timer(Timestamp time, const TimerCallback &timerCallback)
  : time_(time),
    intervalMs_(0),
    callback_(timerCallback),
    id_(s_timerId_.getAndAdd(1))
{
};

bool Timer::next()
{
  if (intervalMs_ <= 0)
    return false;
  time_.add(intervalMs_);
  return true;
};

}