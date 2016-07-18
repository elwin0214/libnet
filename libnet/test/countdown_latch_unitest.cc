#include <libnet/countdown_latch.h>
#include <libnet/logger.h>
#include <libnet/thread.h>
#include <assert.h>
#include <functional>

using namespace libnet;

CountDownLatch countDownLatch(2);


void thread_wait()
{
  countDownLatch.wait();
  LOG_INFO << "finished "; 
  assert(countDownLatch.count() == 0);
};

void thread_countDown()
{
  countDownLatch.countDown();
};


int main()
{

  Thread t1(std::bind(thread_wait));

  Thread t21(std::bind(thread_countDown));
  Thread t22(std::bind(thread_countDown));

  t1.start();
  t21.start();
  t22.start();

  t1.join();
  t21.join();
  t22.join();

  return 0;
}