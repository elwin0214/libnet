#ifndef __EXAMPLE_ECHO_SERVER_H__
#define __EXAMPLE_ECHO_SERVER_H__

#include <libnet/server.h>
#include <libnet/eventloop.h>

using namespace libnet;

class EchoServer
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

public:
  EchoServer(EventLoop* loop, const char* host, int port, int threads);

  void start();

  void onConnection(const ConnectionPtr& connection);

  void onMessage(const ConnectionPtr& connection);

private:
  EventLoop* loop_;
  Server server_; 

};

#endif