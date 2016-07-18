#include "eventloop_group.h"
#include "eventloop.h"
#include "eventloop_thread.h"

namespace libnet
{

EventLoopGroup::EventLoopGroup(EventLoop* baseLoop, int num, const std::string& name)
  : baseLoop_(baseLoop),
    name_(name),
    index_(0),
    num_(num)
{
    
};

void EventLoopGroup::start()
{
  baseLoop_->assertInLoopThread();
  for (int i = 0; i < num_; i++)
  {
    std::string threadName = name_;
    threadName.push_back('-');
    threadName.push_back(i + '1' -1);
    EventLoopThreadPtr loopThreadPtr(new EventLoopThread(threadName));
    eventLoopThreads_.push_back(loopThreadPtr);
    loopThreadPtr->start();
  }    
};

EventLoop* EventLoopGroup::getNextLoop()
{
  baseLoop_->assertInLoopThread();
  if (num_ <= 0)
    return baseLoop_;
  int index = index_++ % num_;
  return eventLoopThreads_[index]->getLoop();
};

}