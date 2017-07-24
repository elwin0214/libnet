#ifndef __LIBNET_CURRENTTHREAD_H__
#define __LIBNET_CURRENTTHREAD_H__

#include <pthread.h>
#include <sys/syscall.h>

#ifdef __linux__
#define TID pid_t
#else
#define TID pthread_t
#endif

namespace libnet
{
namespace thread
{

extern __thread const char* t_threadName;
extern __thread TID t_tid;

TID currentTid();
const char* currentThreadName();

}
}

#endif