#include <stdlib.h>
#include <sys/syscall.h>
#include <exception>
#include <libnet/thread.h>
#include <libnet/logger.h>
#include <libnet/exception.h>

namespace libnet
{
namespace thread
{

__thread const char* t_threadName = "";
__thread TID t_tid = 0; 

TID currentTid()
{
  if (0 == t_tid)
  {
    #ifdef __linux__
    t_tid = ::syscall(SYS_gettid);
    #else
    t_tid = ::pthread_self();
    #endif
  }
  return t_tid;
};

const char* currentThreadName()
{
  return t_threadName;
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
    LOG_DEBUG << "tid = " << tid_;
  else
    LOG_SYSFATAL << "pthread_create!" ;
};

void Thread::run()
{
  if (Thread::initCallback_)
  {
    Thread::initCallback_();
  }
  if ("" != name_)
    thread::t_threadName = name_.c_str();
  else 
    thread::t_threadName = "main";

  #ifdef __linux__
  pid_ = ::syscall(SYS_gettid);
  thread::t_tid = pid_;
  #else
  thread::t_tid = tid_;
  #endif

  try
  {
    func_();
  }
  catch(const Exception& e)
  {
    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", e.message());
    fprintf(stderr, "stack trace: %s\n", e.stackTrace());
    abort();
  }
  catch(const std::exception& e)
  {
    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", e.what());
    abort();
  }
  catch(...)
  {
    fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
    abort();
  }
};

void Thread::join()
{
  if (started_ && !joined_) 
  {
    joined_ = true;
    int r = pthread_join(tid_, NULL);
    if (r == 0)
      LOG_TRACE << "tid = " << tid_;
    else
      LOG_SYSERROR << "tid = " << tid_ << " pthread_join!";
  }  
};

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    int r = pthread_detach(tid_);
    if (r == 0)
      LOG_TRACE << "pthread_detach tid = " << tid_;
    else
      LOG_SYSERROR << "pthread_detach! tid = " << tid_;
  }

};
Thread::ThreadFunc Thread::initCallback_ = std::function<void()>();
}