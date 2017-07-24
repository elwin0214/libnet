#ifndef __EXAMPLE_ECHO_SERVER_H__
#define __EXAMPLE_ECHO_SERVER_H__

#include <libnet/server.h>
#include <libnet/eventloop.h>

using namespace libnet;

class EchoServer
{
public:
  typedef std::shared_ptr<Connection> Conn;

public:
  EchoServer(EventLoop* loop, const char* host, uint16_t port);

  void start();

  void onConnection(const Conn& conn);

  void onMessage(const Conn& conn);

private:
  EventLoop* loop_;
  Server server_; 

};

#endif