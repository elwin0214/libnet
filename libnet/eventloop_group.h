#ifndef __LIBNET_EVENTLOOPGROUOP_H__
#define __LIBNET_EVENTLOOPGROUOP_H__

#include <vector>
#include <string>
#include <memory>
#include "nocopyable.h"

namespace libnet
{

class EventLoop;
class EventLoopThread;

class EventLoopGroup : public NoCopyable 
{

public:
  typedef std::shared_ptr<EventLoopThread> EventLoopThreadPtr;

public:
  EventLoopGroup(EventLoop* baseLoop, int num, const std::string& name);
    
  void start();

  EventLoop* getNextLoop(); 

private:
  EventLoop* baseLoop_;
  std::string name_;
  int index_;
  int num_;
  std::vector<EventLoopThreadPtr> eventLoopThreads_;

};

}

#endif