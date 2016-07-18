#ifndef __EXAMPLE_ECHO_SERVER_H__
#define __EXAMPLE_ECHO_SERVER_H__

#include <libnet/server.h>
#include <libnet/eventloop.h>

using namespace libnet;

class EchoServer
{
public:
  EchoServer(EventLoop* loop, const char* host, int port, int threadNum);

  void start();

  void onConnection(std::shared_ptr<Connection> conPtr);

  void onMessage(std::shared_ptr<Connection> conPtr);

private:
  EventLoop* loop_;
  Server server_; 

};

#endif