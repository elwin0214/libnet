#include <functional>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <libnet/logger.h>
#include <libnet/connection.h>
#include <libnet/cstring.h>
#include "echo_server.h"

EchoServer::EchoServer(EventLoop* loop, const char* host, int port, int threadNum)
    : loop_(loop),
      server_(loop, host, port, threadNum)
{
};

void EchoServer::start()
{ 
  server_.setConnectionCallBack(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&EchoServer::onMessage, this, std::placeholders::_1));
  server_.start();
};

void EchoServer::onConnection(std::shared_ptr<Connection> connection)
{
  LOG_INFO << "connection id=" << (connection->id())  <<" state=" << (connection->stateToString());

  if (connection->connected())
  {
    LOG_INFO << "connection id=" << (connection->id()) << " connected";
  }
  else if (connection->disconnected())
  {
    LOG_INFO << "connection id=" << (connection->id()) << " disconnected";
  }
};

void EchoServer::onMessage(std::shared_ptr<Connection> connection)
{
  std::string str = connection->input().toString();
  int id = connection->id();
  LOG_INFO << "connection id="<< id << " message=" << str;
  
  if (str == "exit\r\n")
  {
    LOG_INFO << "connection id="<< id << " go to close.";
    connection->shutdown();
    return;
  }
  else if (str == "exit all\r\n")
  {
    LOG_INFO << "server goto close.";
    loop_->shutdown();
    loop_->wakeup();
    return;
  }
  //std::shared_ptr<Buffer> echo(new Buffer(1024));
  //echo->append(conPtr->input());
  connection->input().clear();
  connection->send(str.data(), str.size());
};
