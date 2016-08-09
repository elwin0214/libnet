#ifndef __LIBNET_THREAD_THREADPOOL_H__
#define __LIBNET_THREAD_THREADPOOL_H__
 
#include <queue>
#include <memory> 
#include "nocopyable.h"
#include "condition.h"
#include "mutexlock.h"
#include "thread.h"

namespace libnet
{

class ThreadPool : public NoCopyable
{
public:
  typedef std::function<void()>  Task;

private:
  std::queue<Task> queue_;
  std::vector<std::shared_ptr<Thread>> threads_;
  size_t threadSize_;
  size_t maxQueueSize_;

  bool stop_;

  MutexLock lock_;
  Condition notEmpty_;
  Condition notFull_;

private:
  void runInThread();
  Task take();
  bool isFull();

public:
  ThreadPool(size_t threadSize, size_t maxQueueSize);
    
  void start();

  void add(Task task);

  void stop();

  void wait();

  ~ThreadPool();

};

}

#endif