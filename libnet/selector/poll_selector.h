#ifndef __LIBNET_POLLSELECTOR_H__
#define __LIBNET_POLLSELECTOR_H__

#include <poll.h>
#include "selector.h"

namespace libnet
{
class Channel;
class EventLoop;
namespace selector
{

class PollSelector : public Selector
{
public:
  typedef std::vector<struct pollfd> PollFds;

  PollSelector(EventLoop *loop);
  virtual ~PollSelector();
  //virtual void addChannel(Channel *ch);
  virtual void updateChannel(Channel *ch);
  virtual void removeChannel(Channel *ch);

  virtual void selectNow(ChannelList& activeChannles);
  virtual void select(ChannelList& activeChannles);
  virtual void select(int timeoutMs, ChannelList& activeChannles);

private:
  PollFds  pollfds_;
  int index_;
};
 

}
}

#endif