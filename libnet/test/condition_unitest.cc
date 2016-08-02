#include <libnet/mutexlock.h>
#include <libnet/condition.h>
#include <libnet/thread.h>
#include <queue>
#include <vector>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include <memory>

using namespace std;
using namespace libnet;

int g_consume = 0;

class ProducerAndConsumer
{

public:
  ProducerAndConsumer(int capcity)
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

  void consume()
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
        int value = queue_.front();
        queue_.pop();
        g_consume += value;
        nofull_.notifyAll();
        break;
      }
    }
  }

 


private:
  MutexLock lock_;
  Condition noempty_;
  Condition nofull_;
  int capcity_;
  queue<int> queue_;

};

int main()
{
  ProducerAndConsumer pc(1);

  // auto func = std::bind(&ProducerAndConsumer::produce, &pc, 1);
  // func();

  vector<shared_ptr<Thread>> producers;
  vector<shared_ptr<Thread>> consumers;


  for (int i = 0; i < 10; i++)
  {
    shared_ptr<Thread> p = shared_ptr<Thread>(new Thread(std::bind(&ProducerAndConsumer::produce,  &pc, 1)));
    producers.push_back(p);
    shared_ptr<Thread> c = shared_ptr<Thread>(new Thread(std::bind(&ProducerAndConsumer::consume, &pc)));
    consumers.push_back(c);
    p->start();
    c->start();
  }

  for (auto thread : producers)
  {

    thread->join();
  }

  for (auto thread : consumers)
  {
    thread->join();
  }
  cout << g_consume << endl;
  assert (g_consume == 10);
}

