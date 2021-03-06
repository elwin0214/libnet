#include <sys/time.h>
#include <libnet/timestamp.h>

namespace libnet
{

Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return Timestamp(tv.tv_sec * 1000000 + tv.tv_usec);
};

std::string Timestamp::toString() const
{
  char buf[32];
  time_t seconds = static_cast<time_t>(micro_seconds_ / kMicroSecondsPerSecond);
  struct tm time;
  ::localtime_r(&seconds, &time);
  snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
             time.tm_hour, time.tm_min, time.tm_sec);
  return buf;
};

}