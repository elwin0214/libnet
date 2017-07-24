#ifndef __LIBNET_THREAD_H__
#define __LIBNET_THREAD_H__

#include <string>
#include <atomic>
#include <pthread.h>
#include <functional>
#include <sys/types.h>
#include "nocopyable.h"
#include "current_thread.h"

namespace libnet
{

class Thread : public NoCopyable
{
public:
  typedef std::function<void()>  ThreadFunc;

public:
  Thread(const ThreadFunc &func);
  Thread(const ThreadFunc &func, const std::string &name);
    
  TID tid() 
  {
    #ifdef __linux__ 
    return pid_; 
    #else
    return tid_;
    #endif
  }
    
  void start();
  virtual void run();
  void join(); 
  virtual ~Thread();
  static void registerInitCallback(std::function<void()> func) { Thread::initCallback_ = func; }

protected:
    std::atomic<bool> started_;
    std::atomic<bool> joined_;
    
private:
    ThreadFunc func_;
    std::string name_;
    pthread_t tid_;
    pid_t pid_;
    static ThreadFunc initCallback_;
};

}

#endif
