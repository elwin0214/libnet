#include "eventloop_group.h"
#include "eventloop.h"
#include "eventloop_thread.h"

namespace libnet
{
using namespace std;
EventLoopGroup::EventLoopGroup(EventLoop* loop, int num, const std::string& name)
  : base_loop_(loop),
    name_(name),
    index_(0),
    num_(num),
    loop_threads_()
{
    loop_threads_.reserve(num);
};

void EventLoopGroup::start()
{
  base_loop_->assertInLoopThread();
  for (int i = 0; i < num_; i++)
  {
    std::string name = name_;
    name.push_back('-');
    name.push_back(i + '1' -1);
    LoopThread thread = make_shared<EventLoopThread>(name);
    loop_threads_.push_back(thread);
    thread->start();
  }    
};

EventLoop* EventLoopGroup::getNextLoop()
{
  base_loop_->assertInLoopThread();
  if (num_ <= 0)
    return base_loop_;
  int index = index_++ % num_;
  return loop_threads_[index]->getLoop();
};

}