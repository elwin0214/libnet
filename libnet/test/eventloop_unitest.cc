#include <iostream>
#include <unistd.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <assert.h>

using namespace libnet;
 
void f()
{
    
};

int main()
{
  EventLoopThread thread("loop");
  thread.start();
  EventLoop* loop = thread.getLoop();
  assert(NULL != loop);

  loop->runInLoop(std::bind(f));
     
    return 0;
}