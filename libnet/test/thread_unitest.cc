#include <stdio.h>  
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>  
#include <assert.h>
#include <libnet/thread.h>
#include <libnet/current_thread.h>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
using namespace libnet;

pid_t gettid()
{
     return syscall(SYS_gettid);  
};
int gInited = 0;
struct TestThread
{
  pthread_t tid_;

  void f()
  {
    tid_ = thread::currentTid();
  }

  void init()
  {
    gInited = 1;
  }
};


TEST(Thread, init)
{
  TestThread t;
  Thread::registerInitCallback(std::bind(&TestThread::init, &t));

  Thread thread(std::bind(&TestThread::f, &t), "test");
  thread.start();
  thread.join();
  cout << t.tid_  << " "  << thread.tid() ;
  ASSERT_EQ(t.tid_, thread.tid());
  ASSERT_EQ(gInited, 1);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}