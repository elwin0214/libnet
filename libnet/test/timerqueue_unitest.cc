#include <sys/time.h>
#include <iostream>
#include <functional>
#include <map>
#include <unistd.h>
#include <libnet/timer.h>
#include <libnet/timestamp.h>
#include <libnet/timer_queue.h>
#include <libnet/eventloop.h>

using namespace std;
using namespace libnet;

//TimerQueue* g_timerQueue;
EventLoop* g_loop;
//int count = 0;

void func(const char* msg)
{
  fprintf(stdout, "msg=%s,now=%s\n", msg, Timestamp::now().toString().c_str());
};

void cycle_func(const char* msg)
{
  static int count = 0;
  fprintf(stdout, "msg=%s,now=%s\n", msg, Timestamp::now().toString().c_str());
  if (++count > 4)
  {
    g_loop->shutdown();
  }
};

void cancel(const TimerId& timerId)
{
   g_loop->cancel(timerId);
};


 
void test_normal()
{  
  EventLoop loop;

  g_loop = &loop;

  loop.runInterval(1000, 2000, std::bind(cycle_func, "cycle 2s task"));
  loop.runAfter(2000, std::bind(func, "task after 2s"));
  loop.runAfter(3000, std::bind(func, "task after 3s"));
  loop.runAfter(5000, std::bind(func, "task after 5s"));
  loop.runAfter(6000, std::bind(func, "task after 6s"));
  loop.runAfter(10000, std::bind(func, "wont run task"));

  loop.wakeup();
  loop.loop();
};


int main()
{
  //setLogLevel(log::TRACE);
  test_normal();
  return 0;
}