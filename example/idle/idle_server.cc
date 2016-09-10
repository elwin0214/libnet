#include <libnet/eventloop.h>
#include <libnet/buffer.h>
#include <string>
#include "idle_server.h"

void IdleServer::start()
{ 
  server_.setConnectionCallBack(std::bind(&IdleServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&IdleServer::onMessage, this, std::placeholders::_1));
  server_.start();
  timerId_ = loop_->runInterval(1, 1000, std::bind(&IdleServer::onTimer, this));
}

void IdleServer::onConnection(const ConnectionPtr& connection)
{
  timewheel_.onConnection(connection);
}

void IdleServer::onMessage(const ConnectionPtr& connection)
{
  timewheel_.onMessage(connection);
  Buffer& input = connection->input();
  const char* end = input.find("\r\n");
  if (end != NULL)
  {
    const char* start = input.beginRead();
    std::string line = std::string(start, end - start);
    input.moveReadIndex(end - start + 2);
    if (line == "exit")
    {
      connection->shutdown();
    }
    else if (line == "exit_all")
    {
      loop_->shutdown();
    }
    else
    {
      connection->send(line);
      connection->send("\r\n");
    }
  }
}

void IdleServer::onTimer()
{
  timewheel_.rotate();
}

IdleServer::~IdleServer()
{
  if (timerId_)
    loop_->cancel(timerId_);
}
