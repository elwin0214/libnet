#ifndef __LIBNET_EVENTLOOPGROUOP_H__
#define __LIBNET_EVENTLOOPGROUOP_H__

#include <vector>
#include <string>
#include <memory>
#include <libnet/nocopyable.h>
#include <libnet/logger.h>

namespace libnet
{

class EventLoop;
class EventLoopThread;

class EventLoopGroup : public NoCopyable 
{

public:
  typedef std::shared_ptr<EventLoopThread> LoopThread;

public:
  EventLoopGroup(EventLoop* loop, int num, const std::string& name);
    
  void start();

  EventLoop* getNextLoop(); 

  ~EventLoopGroup()
  {
    LOG_TRACE  << "~EventLoopGroup()" ;
  }

private:
  EventLoop* base_loop_;
  std::string name_;
  int index_;
  int num_;
  std::vector<LoopThread> loop_threads_;

};

}

#endif