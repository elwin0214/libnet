#ifndef __LIBNET_SELECTOR_H__
#define __LIBNET_SELECTOR_H__

#include "../nocopyable.h"
#include <vector>
#include <map>

namespace libnet
{
class Channel;
class EventLoop;
namespace selector
{

class Selector : public NoCopyable 
{

public:
  typedef std::vector<Channel*> ChannelList;

protected:
  EventLoop *loop_;
  std::map<int, Channel*> channels_;
  

public:
  Selector(EventLoop* loop);
    virtual ~Selector();
    //virtual void select(int timeoutMs, ChannelList& activeChannles) = 0;

    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    virtual void selectNow(ChannelList& activeChannles) = 0;
    virtual void select(ChannelList& activeChannles) = 0;
    virtual void select(int timeoutMs, ChannelList& activeChannles) = 0;

};

}
}
#endif