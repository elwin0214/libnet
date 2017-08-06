#include <libnet/condition.h>

namespace libnet
{

int64_t Condition::wait(uint32_t wait_milli)
{
  Timestamp before = Timestamp::now();
  before.add(wait_milli);
  struct timespec ts = before.getTimespec();
  //or if the system time reaches the time specified in abstime, and the current
  //thread reacquires the lock on mutex.
  pthread_cond_timedwait(&cond_, &(lock_.mutex), &ts);
  Timestamp after = Timestamp::now();
  return after.sub(before) / 1000;
}

}