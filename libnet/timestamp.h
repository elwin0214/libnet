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

  void add(int timeMs);

  int64_t value() const
  {
    return microSeconds_;
  }

  bool operator< (const Timestamp& ts) const
  {
    return microSeconds_ < ts.microSeconds_;
  }

  std::string toString() const;

  static const int kMicroSecondsPerSecond = 1000 * 1000;

private:
  int64_t microSeconds_; //0.0000001s
  
};

}

#endif