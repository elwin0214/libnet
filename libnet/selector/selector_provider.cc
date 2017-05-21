#include <stdlib.h>
#include <string.h>
#include "selector_provider.h"
#include "default_selector.h"
#include "poll_selector.h"
#ifdef HAVE_EPOLL
#include "epoll_selector.h"
#endif

#include "../logger.h"

namespace libnet
{
namespace selector
{

Selector* SelectorProvider::provide(EventLoop *loop)
{
  const char* env = ::getenv("SELECTOR");

  if (NULL == env)
  {
    LOG_INFO << "can not find env, use default" ;
    return new PollSelector(loop);
  }
  LOG_INFO << "select = " << env ;
  if (strcmp(env, "select") == 0)
  {
    return new DefaultSelector(loop);
  }
  if (strcmp(env, "epoll") == 0)
  {
    #ifdef HAVE_EPOLL
    return new EpollSelector(loop);
    #else
    return new PollSelector(loop);
    #endif
  }
  else if (strcmp(env, "poll") == 0)
  {
    return new PollSelector(loop);
  }
  else
  {
    LOG_INFO << "env-" << env <<", use default";
    return new DefaultSelector(loop);
  }
  
};

}
}