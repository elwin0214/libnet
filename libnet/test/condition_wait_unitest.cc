#include <libnet/mutexlock.h>
#include <libnet/condition.h>
#include <libnet/thread.h>
#include <libnet/logger.h>
#include <gtest/gtest.h>
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

void setValue()
{
  sleep(1);
  LockGuard guard(g_Lock);
  g_value = 1;
  g_condition.notifyAll();
}

TEST(Condition, wait)
{
  waitForGet();
  ASSERT_EQ(g_value, 0);

  Thread thread(std::bind(setValue));
  thread.start();
  waitForGet();
  ASSERT_EQ(g_value, 1);

  thread.join();
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}