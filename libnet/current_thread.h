#ifndef __LIBNET_CURRENTTHREAD_H__
#define __LIBNET_CURRENTTHREAD_H__

#include <pthread.h>

namespace libnet
{
namespace thread
{

extern __thread const char* currentThreadName;

pthread_t currentTid();

}
}

#endif