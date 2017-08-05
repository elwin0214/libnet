#include <atomic>
#include <libnet/timer.h>
#include <libnet/timer_queue.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/countdown_latch.h>
#include <gtest/gtest.h>

using namespace std;
using namespace libnet;

TEST(TimerQueue, run_interval)
{
  int flag = 0;
  EventLoop loop;
  loop.runInterval(100, 200, [&flag](){
    flag++;
  });
  loop.runAfter(550, [&loop](){
    loop.shutdown();
  });
  loop.loop();
  ASSERT_TRUE(flag == 3) << "flag is " << flag;
}


TEST(TimerQueue, run_interval_otherthread)
{
  std::atomic<int> flag(0);
  EventLoopThread loop_thread("loop");
  loop_thread.start();
  EventLoop* loop = loop_thread.getLoop();
  loop->runInterval(100, 200, [&flag](){
    flag.fetch_add(1);
  });

  CountDownLatch latch(1);
  loop->runAfter(550, [&loop, &latch](){
    loop->shutdown();
    latch.countDown();
  });
  latch.wait();
  ASSERT_TRUE(flag.load() == 3) << "flag is " << flag.load();
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}