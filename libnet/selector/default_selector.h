#ifndef __LIBNET_DEFAULTSELECTOR_H__
#define __LIBNET_DEFAULTSELECTOR_H__

#include <errno.h>
#include <sys/select.h>
#include "selector.h"

namespace libnet
{
class Channel;

namespace selector
{

class DefaultSelector : public Selector
{
public:
  DefaultSelector(EventLoop *loop);
  ~DefaultSelector();
  //virtual 
  virtual void updateChannel(Channel *channel);
  virtual void removeChannel(Channel *channel);

  virtual void selectNow(ChannelList& activeChannles);
  virtual void select(ChannelList& activeChannles);
  virtual void select(int timeoutMs, ChannelList& activeChannles);

private:
  void selectWithTimeval(struct timeval *tvPtr, ChannelList& activeChannles);
  //void addChannel(Channel *channel);
  void updateChannel(int events, Channel *channel);
private:
  int max_fd_;
  //int index_;
  fd_set read_fs_;
  fd_set write_fs_;
  fd_set error_fs_;
};

}
}

#endif