#ifndef __LIBNET_TIMER_H__
#define __LIBNET_TIMER_H__
#include <functional>
#include "timestamp.h"
#include "nocopyable.h"
#include "atomic.h"
#include "logger.h"

namespace libnet
{

class Timer : public NoCopyable
{

public:
  typedef std::function<void()>  TimerCallback;

  Timer(Timestamp time, const TimerCallback &callback);

  Timer(Timestamp time, TimerCallback &&callback);

  Timer(Timestamp time, int inerval, const TimerCallback& timerCallback);

  Timer(Timestamp time, int inerval, TimerCallback&& timerCallback);

  ~Timer()
  {
    LOG_TRACE << "destroy!" ;
  }

  Timestamp& time(){ return time_; }

  const TimerCallback& callback() {return callback_; }

  void run() {callback_(); }

  bool next();

  int64_t id() {return id_; }

private:
  Timestamp time_;
  int interval_; //ms
  const TimerCallback callback_;
  int64_t id_;
  static AtomicInt64 s_timerId_;

};

class TimerId
{
public:
  TimerId(Timer* timer, int64_t id):timer_(timer),id_(id)
  {

  }
  
  bool operator< (const TimerId& timerId) const
  {
    return id_ < timerId.id_;
  }

  Timer* timer() {return timer_;}
  int64_t id() {return id_;}

private:
  Timer* timer_;
  int64_t id_;
};

}

#endif