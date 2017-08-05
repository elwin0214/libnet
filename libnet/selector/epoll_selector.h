#ifndef __LIBNET_EPOLLSELECTOR_H__
#define __LIBNET_EPOLLSELECTOR_H__

#include <sys/epoll.h>
#include "selector.h"

namespace libnet
{
class Channel;

namespace selector
{

class EpollSelector : public Selector
{
public:
  typedef std::vector<struct epoll_event> EpollEvents;

  EpollSelector(EventLoop *loop);
  virtual ~EpollSelector();

  virtual void updateChannel(Channel *ch);
  virtual void removeChannel(Channel *ch);

  virtual void selectNow(ChannelList& activeChannles);
  virtual void select(ChannelList& activeChannles);
  virtual void select(int timeoutMs, ChannelList& activeChannles);

private:
  void updateChannel(int op, Channel *ch);

private:
  int epfd_;
  EpollEvents epollEvents_;
};

}
}

#endif