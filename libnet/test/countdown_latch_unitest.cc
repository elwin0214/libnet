#include <libnet/countdown_latch.h>
#include <libnet/logger.h>
#include <libnet/thread.h>
#include <assert.h>
#include <functional>
#include <vector>
#include <memory>

using namespace std;
using namespace libnet;

int main()
{
  CountDownLatch latch(100);
  std::vector<unique_ptr<Thread>> threads;
  threads.reserve(100);
  for (int i = 0; i < 100; i++)
  {
    unique_ptr<Thread> up(new Thread([&latch](){ latch.countDown(); }));
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
  latch.wait();
  assert(true);
  return 0;
}