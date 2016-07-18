#include <stdio.h>  
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>  
#include <assert.h>
#include <libnet/thread.h>
#include <libnet/current_thread.h>
#include <iostream>

using namespace std;
using namespace libnet;

pid_t gettid()
{
     return syscall(SYS_gettid);  
};

struct Test
{
  pthread_t tid_;

  void f()
  {
    tid_ = thread::currentTid();
  }
};

void test_tid()
{
  Test t;
  Thread thread(std::bind(&Test::f, &t), "test");
  thread.start();
  thread.join();
  cout << t.tid_  << " "  << thread.tid() ;
  assert(t.tid_ == thread.tid());
}

int main()
{
  test_tid();
  return 0;
}