#ifndef __LIBNET_THREAD_H__
#define __LIBNET_THREAD_H__

#include "nocopyable.h"
#include <string>
#include <pthread.h>
#include <functional>
#include <sys/syscall.h>
#include <sys/types.h>

namespace libnet
{

//void* thread_execute(void*);

class Thread : public NoCopyable
{
public:
    typedef std::function<void()>  ThreadFunc;

public:
    Thread(const ThreadFunc &func);
    Thread(const ThreadFunc &func, const std::string &name);
    
    pthread_t tid() {return tid_; }
    void start();
    virtual void run();
    void join();
    virtual ~Thread();

protected:
    bool started_;
    bool joined_;
    
private:
    ThreadFunc func_;
    std::string name_;
    pthread_t tid_;
    

};

}

#endif
