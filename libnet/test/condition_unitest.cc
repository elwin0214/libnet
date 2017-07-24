#include <libnet/mutexlock.h>
#include <libnet/condition.h>
#include <libnet/thread.h>
#include <gtest/gtest.h>
#include <queue>
#include <vector>
#include <assert.h>
#include <memory>

using namespace std;
using namespace libnet;

template<typename T>
class SyncQueue
{

public:
  SyncQueue(int capcity)
    : lock_(),
      noempty_(lock_),
      nofull_(lock_),
      capcity_(capcity),
      queue_()
  {

  }

  void produce(int value)
  {
    LockGuard guard(lock_);
    while(true)
    {
      if (queue_.size() >= capcity_)
      {
        nofull_.wait();
      }
      else
      {
        queue_.push(value);
        noempty_.notifyAll();
        break;
      }
    }
  }

  T consume()
  {
    LockGuard guard(lock_);
    while(true)
    {
      if (queue_.size() <= 0)
      {
        noempty_.wait();
      }
      else
      {
        T& t = queue_.front();
        T result = std::move(t);
        queue_.pop();  
        nofull_.notifyAll();
        return result;
      }
    }
  }

private:
  MutexLock lock_;
  Condition noempty_;
  Condition nofull_;
  size_t capcity_;
  queue<T> queue_;

};

TEST(Condition, test)
{
  SyncQueue<int> cache(10);
  std::vector<int> values;
  values.reserve(100);
  std::vector<unique_ptr<Thread>> threads;
  threads.reserve(10);

  for (int i = 0; i < 10; i++)
  {
    unique_ptr<Thread> up(new Thread([&cache, i]()mutable{ 
      for (int j = 0; j < 10; j++)
      {
        cache.produce(j + i * 10);
      }
    }));
    threads.push_back(std::move(up));
  }

  Thread consumer([&values, &cache](){
    for (int i = 0; i < 100; i++)
    {
      int v = cache.consume();
      values.push_back(v);
    }
  });

  consumer.start();
  for (auto& t : threads)
    t->start();

  consumer.join();
  for (auto& t : threads)
    t->join();
  
  std::sort(values.begin(), values.end());

  int index = 0;
  assert(values.size() == 100);
  for (auto value : values)
  { 
    assert(value == index++);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}