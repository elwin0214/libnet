#ifndef __EXAMPLE_IDLE_SERVER_H__
#define __EXAMPLE_IDLE_SERVER_H__

#include <libnet/timewheel.h>
#include <libnet/defs.h>
#include <libnet/server.h>
#include <libnet/timer.h>

using namespace libnet;
//class EventLoop;
class IdleServer
{
public:
  IdleServer(EventLoop* loop, const char* host, int port, size_t seconds)
    : loop_(loop),
      server_(loop, host, port, 0), //single thread server
      timewheel_(seconds),
      timerId_()
  {

  }

  void start();

  void onConnection(const ConnectionPtr& connection);

  void onMessage(const ConnectionPtr& connection);

  void onTimer();

  ~IdleServer();

private:
  EventLoop* loop_;
  Server server_;
  TimeWheel timewheel_;
  TimerId timerId_;

};

#endif