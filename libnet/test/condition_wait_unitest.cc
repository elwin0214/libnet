#include <libnet/mutexlock.h>
#include <libnet/condition.h>
#include <libnet/thread.h>
#include <libnet/logger.h>
#include <assert.h>
#include <unistd.h>

using namespace std;
using namespace libnet;

MutexLock g_Lock;
Condition g_condition(g_Lock);
int g_value = 0;

void waitForGet()
{
  LockGuard guard(g_Lock);
  g_condition.wait(3000);
}

void set()
{
  sleep(1);
  LockGuard guard(g_Lock);
  g_value = 1;
  g_condition.notifyAll();
}

int main()
{
  LOG_INFO << " begin to wait 3s " ;
  waitForGet();
  LOG_INFO << " value = " << g_value ;
  assert (g_value == 0);

  Thread thread(std::bind(set));
  thread.start();
  waitForGet();
  LOG_INFO << " value = " << g_value ;
  assert (g_value == 1);

  thread.join();
}