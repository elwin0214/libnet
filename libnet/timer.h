#ifndef __LIBNET_TIMER_H__
#define __LIBNET_TIMER_H__
#include <functional>
#include <libnet/timestamp.h>
#include <libnet/nocopyable.h>
#include <libnet/logger.h>
#include <atomic>

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

  const Timestamp& time(){ return time_; } const

  //Timestamp& time(){ return time_; }

  TimerCallback& callback() const {return callback_; }

  void run() {callback_(); }

  bool next();

  uint64_t id() {return id_; }

private:
  Timestamp time_;
  int interval_; //ms the period of the timer which will be run.it's only valid when greater than 0
  const TimerCallback callback_;
  uint64_t id_;
  static std::atomic<uint64_t>  g_timer_id_;

};

class TimerId
{
public:
  TimerId():timer_(NULL),id_(-1)
  {

  }

  TimerId(Timer* timer, uint64_t id):timer_(timer),id_(id)
  {

  }

  operator bool() const
  {
    return NULL != timer_;
  }

  bool operator< (const TimerId& timerId) const
  {
    return id_ < timerId.id_;
  }

  Timer* timer() {return timer_;}
  uint64_t id() {return id_;}

private:
  Timer* timer_;
  uint64_t id_;
};

}

#endif