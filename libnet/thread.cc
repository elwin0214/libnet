#include <errno.h> 
#include "thread.h"
#include "logger.h"
#include "current_thread.h"

namespace libnet
{
namespace thread
{

__thread const char* currentThreadName = "";

pthread_t currentTid()
{
    return pthread_self();
};

void* thread_execute(void* param)
{
  Thread *thread = static_cast<Thread*>(param);
  thread->run();
  return NULL;
};

}

Thread::Thread(const ThreadFunc &func)
    : started_(false),
      joined_(false),
      func_(func),
      name_("")
{

};
Thread::Thread(const ThreadFunc &func,const std::string &name)
    : started_(false),
      joined_(false),
      func_(func),
      name_(name)
{

};


void Thread::start()
{
  if (started_) 
    return;
  started_ = true;
  int r = pthread_create(&tid_, NULL, thread::thread_execute, this);
  if (r == 0)
    LOG_DEBUG << "tid=" << tid_;
  else
    LOG_SYSFATAL << "pthread_create!" ;
};

void Thread::run()
{

  if ("" != name_)
    thread::currentThreadName = name_.c_str();
  else 
    thread::currentThreadName = "main";

  func_();
};

void Thread::join()
{
  if (started_ && !joined_) 
  {
    joined_ = true;
    int r = pthread_join(tid_, NULL);
    if (r == 0)
      LOG_TRACE << "tid=" << tid_;
    else
      LOG_SYSERROR << "tid=" << tid_ << "pthread_join!";
  }  
};

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    int r = pthread_detach(tid_);
    if (r == 0)
      LOG_TRACE << "pthread_detach tid=" << tid_;
    else
      LOG_SYSERROR << "pthread_detach! tid=" << tid_;
  }

};

}