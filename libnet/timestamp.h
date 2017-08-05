#ifndef __LIBNET_TIMESTAMP_H__
#define __LIBNET_TIMESTAMP_H__

#include <string>
#include <stdlib.h>

namespace libnet
{

class Timestamp
{

public:
  explicit Timestamp(int64_t microSeconds)
    :  microSeconds_(microSeconds)
  {

  }

  static Timestamp now();

  uint64_t value() const
  {
    return microSeconds_;
  };

  uint64_t secondsValue() const 
  {
    return microSeconds_ / kMicroSecondsPerSecond;
  };

  uint64_t milliSecondsValue() const
  {
    return microSeconds_ / kMilliSecondPerSecond;
  };

  bool operator< (const Timestamp& ts) const
  {
    return microSeconds_ < ts.microSeconds_;
  };

  void add(int ms)
  {
    microSeconds_ = microSeconds_ + ms * 1000;
  };

  struct timespec getTimespec()
  {
    struct timespec ts;
    ts.tv_sec = microSeconds_ / kMicroSecondsPerSecond;
    ts.tv_nsec = microSeconds_ % kMicroSecondsPerSecond;
    return ts;
  };

  std::string toString() const;

  static const int kMicroSecondsPerSecond = 1000 * 1000;

  static const int kMilliSecondPerSecond = 1000;

private:
  int64_t microSeconds_; //0.0000001s
  
};

}

#endif