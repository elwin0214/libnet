#include <libnet/condition.h>
#include <iostream>

namespace libnet
{

using namespace std;

int64_t Condition::wait(uint32_t wait_milli)
{
  Timestamp until = Timestamp::now();
  until.add(wait_milli);
  struct timespec ts = until.getTimespec();
  //or if the system time reaches the time specified in abstime, and the current
  //thread reacquires the lock on mutex.
  pthread_cond_timedwait(&cond_, &(lock_.mutex), &ts);
  Timestamp now = Timestamp::now();
  //cout << before.value() << " " << after.value() << endl;
  return until.sub(now) / 1000;
}

}