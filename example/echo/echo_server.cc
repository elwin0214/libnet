#include <functional>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <libnet/logger.h>
#include <libnet/connection.h>
#include <libnet/cstring.h>
#include "echo_server.h"

EchoServer::EchoServer(EventLoop* loop, const char* host, uint16_t port)
    : loop_(loop),
      server_(loop, host, port)
{
};

void EchoServer::start()
{ 
  server_.setConnectionCallBack(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&EchoServer::onMessage, this, std::placeholders::_1));
  server_.start();
};

void EchoServer::onConnection(const Conn& conn)
{
  LOG_INFO << "conn id=" << (conn->id())  <<" state=" << (conn->stateToString());

  if (conn->connected())
  {
    LOG_INFO << "conn id=" << (conn->id()) << " connected";
  }
  else if (conn->disconnected())
  {
    LOG_INFO << "conn id=" << (conn->id()) << " disconnected";
  }
};

void EchoServer::onMessage(const Conn& conn)
{
  std::string str = conn->input().toString();
  int id = conn->id();
  LOG_INFO << "conn id = "<< id << " message = " << str;
  
  if (str == "exit\r\n")
  {
    LOG_INFO << "conn id = "<< id << " go to close.";
    conn->shutdown();
    return;
  }
  else if (str == "exit all\r\n")
  {
    LOG_INFO << "server goto close.";
    loop_->shutdown();
    return;
  }
  conn->input().clear();
  conn->send(str);
};
