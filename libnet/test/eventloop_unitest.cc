#include <iostream>
#include <unistd.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/logger.h>
#include <assert.h>

using namespace libnet;

int main()
{
  EventLoopThread thread("loop");
  thread.start();
  EventLoop* loop = thread.getLoop();
  assert(NULL != loop);
  int flag = 0;

  for (int i = 0; i < 100; i++)
    loop->runInLoop([&flag](){ flag++; });
  loop->runInLoop([&flag](){ 
    assert(100 == flag); 
    LOG_INFO << "flag is " << flag ;
  });

  return 0;
}