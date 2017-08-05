#include <iostream>
#include <unistd.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/logger.h>
#include <libnet/countdown_latch.h>
#include <gtest/gtest.h>

using namespace libnet;

TEST(EventLoop, run_after_in_loop)
{
  EventLoop loop;
  std::atomic<bool> flag(false);
  loop.runAfter(100, [&flag]()mutable{
    flag = true;
  });

  loop.runAfter(200, [&loop]()mutable{
    loop.shutdown();
  });
  loop.loop();
  ASSERT_TRUE(flag.load());
}

TEST(EventLoop, run_after_out_loop)
{
  EventLoopThread loop_thread("loop");
  loop_thread.start();
  EventLoop* loop = loop_thread.getLoop();
  std::atomic<bool> flag(false);
  CountDownLatch latch(1);
  loop->runAfter(100, [&flag, &latch]()mutable{
    flag = true;
    latch.countDown();
  });
  latch.wait();
  ASSERT_TRUE(flag.load());
}

TEST(EventLoop, run_after_nest)
{
  EventLoopThread loop_thread("loop");
  loop_thread.start();
  EventLoop* loop = loop_thread.getLoop();
  uint64_t start = Timestamp::now().milliSecondsValue();

  std::atomic<int> flag(0);
  CountDownLatch latch(1);
  loop->runAfter(100, [&loop, &flag, &latch]()mutable{
    flag.fetch_add(1);
    loop->runAfter(100, [&loop, &flag, &latch]()mutable{
      flag.fetch_add(1);
      loop->runAfter(100, [&loop, &flag, &latch]()mutable{
        flag.fetch_add(1);
        latch.countDown();
      }); 
    });
  });
  latch.wait();
  uint64_t end = Timestamp::now().milliSecondsValue();

  ASSERT_TRUE(flag.load() == 3);
  ASSERT_TRUE(end - start < 500); //in 500 ms
}

TEST(EventLoop, run_after_cancel)
{
  EventLoopThread loop_thread("loop");
  loop_thread.start();
  EventLoop* loop = loop_thread.getLoop();
  uint64_t start = Timestamp::now().milliSecondsValue();

  std::atomic<int> flag(0);
  std::atomic<bool> cancel(0);

  CountDownLatch latch(1);
  TimerId timer_id = 
    loop->runAfter(200, [&loop, &flag, &latch]()mutable{ //run
      flag.fetch_add(1);
    });

  
  loop->runAfter(100, [&loop, &cancel, &timer_id]()mutable{ //cancel
     loop->cancel(timer_id);
     cancel = true;
  });

  loop->runAfter(300, [&latch]()mutable{
    latch.countDown();
  });

  latch.wait();
  uint64_t end = Timestamp::now().milliSecondsValue();

  ASSERT_TRUE(flag.load() == 0);
  ASSERT_TRUE(cancel.load());
  ASSERT_TRUE(end - start < 500); //in 500 ms
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}