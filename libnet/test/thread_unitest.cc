#include <memory>
#include <libnet/thread.h>
#include <atomic>
#include <gtest/gtest.h>

using namespace std;
using namespace libnet;

TEST(Thread, init_run)
{
  atomic<bool> init(false);
  atomic<bool> run(false);
  TID tid;
  Thread::registerInitCallback([&init]()mutable{ 
    init = true; 
  });
  Thread thread([&run, &tid]()mutable{
    run = true;
    tid = thread::currentTid();
  }, "test");

  thread.start();
  thread.join();
  ASSERT_EQ(tid, thread.tid());
  ASSERT_TRUE(init.load());
  ASSERT_TRUE(run.load());
}

TEST(Thread, current)
{
  atomic<int> count(0);
  vector<unique_ptr<Thread>> threads;
  threads.reserve(100);
  for (int i = 0; i< 100; i++)
  {
    unique_ptr<Thread> up(new Thread([&count]()mutable{ count.fetch_add(1); }, "test"));
    threads.push_back(std::move(up));
  }

  for (auto& t : threads)
  {
    t->start();
  }

  for (auto& t : threads)
  {
    t->join();
  }
  ASSERT_EQ(100, count);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}