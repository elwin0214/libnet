#include "channel.h"
#include "eventloop.h"


namespace libnet
{

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = 1;
const int Channel::kWriteEvent = 1 << 1;
const int Channel::kErrorEvent = 1 << 2;

Channel::Channel(EventLoop *loop, int fd)
    : fd_(fd), 
      index_(-1),
      events_(0),
      revents_(0),
      loop_(loop)
{
    
};

void Channel::handleEvent()
{
  if ((revents_ & kReadEvent) > 0)
  {
    if (readCallBack_) readCallBack_();
  }
  if ((revents_ & kWriteEvent) > 0)
  {
    if (writeCallBack_) writeCallBack_();
  }
  if ((revents_ & kErrorEvent) > 0)
    if (errorCallBack_) errorCallBack_();
  revents_ = 0;
};

void Channel::update()
{
  loop_->updateChannel(this);
};

void Channel::remove()
{
  loop_->removeChannel(this);
};

}