#ifndef __LIBNET_TIMESTAMP_H__
#define __LIBNET_TIMESTAMP_H__

#include <string>
#include <stdlib.h>

namespace libnet
{

class Timestamp
{

public:
  explicit Timestamp(int64_t micro_seconds)
    :  micro_seconds_(micro_seconds)
  {

  }

  static Timestamp now();

  uint64_t value() const
  {
    return micro_seconds_;
  };

  uint64_t secondsValue() const 
  {
    return micro_seconds_ / kMicroSecondsPerSecond;
  };

  uint64_t milliSecondsValue() const
  {
    return micro_seconds_ / kMilliSecondPerSecond;
  };

  bool operator< (const Timestamp& ts) const
  {
    return micro_seconds_ < ts.micro_seconds_;
  };

  void add(int32_t milli_seconds)
  {
    micro_seconds_ = micro_seconds_ + milli_seconds * 1000;
  };

  void addMicro(int32_t micro_seconds)
  {
    micro_seconds_ = micro_seconds_ + micro_seconds; 
  }

  struct timespec getTimespec()
  {
    struct timespec ts;
    ts.tv_sec = micro_seconds_ / kMicroSecondsPerSecond;
    ts.tv_nsec = micro_seconds_ % kMicroSecondsPerSecond;
    return ts;
  };

  int64_t sub(const Timestamp& ts)
  {
    return micro_seconds_ - ts.micro_seconds_;
  }

  std::string toString() const;

  static const int kMicroSecondsPerSecond = 1000 * 1000;

  static const int kMilliSecondPerSecond = 1000;

private:
  int64_t micro_seconds_; //0.0000001s
  
};

}

#endif