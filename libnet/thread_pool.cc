#include <libnet/thread_pool.h>
#include <functional>
#include <memory>
#include <iostream>

namespace libnet
{

ThreadPool::ThreadPool(size_t threadSize, size_t maxQueueSize)
    : threadSize_(threadSize),
      maxQueueSize_(maxQueueSize),
      stop_(false),
      notEmpty_(lock_),
      notFull_(lock_)
{

};

bool ThreadPool::isFull(){
  return queue_.size() >= maxQueueSize_;
};

void ThreadPool::start()
{
  for (int i = 0 ; i < threadSize_; i++)
  {
    std::function<void()> func = std::bind(&ThreadPool::runInThread, this);
    std::shared_ptr<Thread> thread_ptr = std::make_shared<Thread>(func);
    threads_.push_back(thread_ptr);
    thread_ptr->start();
  }  
};

void ThreadPool::add(Task task)
{
  if (threads_.empty())
  {
    task();
  }
  else
  {
    
    LockGuard guard(lock_);
    while (isFull())
    {
      notFull_.wait();
    }
    queue_.push(task);
    notEmpty_.notifyAll();
  }
};

ThreadPool::Task ThreadPool::take()
{
  LockGuard guard(lock_);
  while (queue_.empty() && !stop_)
  {
    notEmpty_.wait();
  }

  ThreadPool::Task task;
  if (!queue_.empty())
  {
    task = queue_.front();
    queue_.pop();
  }
  notFull_.notifyAll();
  return task;
};

void ThreadPool::runInThread()
{
  while (!stop_)
  {
    Task task(take());
      if (task)
        task();
  }
};

void ThreadPool::stop()
{
  {
    LockGuard guard(lock_);
    stop_ = true;
    notEmpty_.notifyAll();
  }
  for (std::shared_ptr<Thread> thread_ptr : threads_)
  {
    thread_ptr->join();
  }
};

void ThreadPool::wait()
{
  for (std::shared_ptr<Thread> thread_ptr : threads_)
  {
    thread_ptr->join();
  }
};

ThreadPool::~ThreadPool()
{
  if (!stop_)
  {
    stop();
  }
};

}