#ifndef __EXAMPLE_IDLE_SERVER_H__
#define __EXAMPLE_IDLE_SERVER_H__

#include <libnet/timewheel.h>
#include <libnet/server.h>
#include <libnet/timer.h>

using namespace libnet;
//class EventLoop;
class IdleServer
{
public:
  typedef std::shared_ptr<Connection> Conn;
public:
  IdleServer(EventLoop* loop, const char* host, int port, size_t seconds)
    : loop_(loop),
      server_(loop, host, port, 0), //single thread server
      timewheel_(seconds),
      timerId_()
  {

  }

  void start();

  void onConnection(const Conn& conn);

  void onMessage(const Conn& conn);

  void onTimer();

  ~IdleServer();

private:
  EventLoop* loop_;
  Server server_;
  TimeWheel timewheel_;
  TimerId timerId_;

};

#endif