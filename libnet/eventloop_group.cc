#include <libnet/eventloop_group.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>

namespace libnet
{
using namespace std;
EventLoopGroup::EventLoopGroup(EventLoop* loop, size_t num, const std::string& name)
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
  for (size_t i = 0; i < num_; i++)
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
  size_t index = index_++ % num_;
  return loop_threads_[index]->getLoop();
};

}