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

void IdleServer::onConnection(const Conn& conn)
{
  timewheel_.onConnection(conn);
}

void IdleServer::onMessage(const Conn& conn)
{
  timewheel_.onMessage(conn);
  Buffer& input = conn->input();
  const char* end = input.find("\r\n");
  if (end != NULL)
  {
    const char* start = input.beginRead();
    std::string line = std::string(start, end - start);
    input.moveReadIndex(end - start + 2);
    if (line == "exit")
    {
      conn->shutdown();
    }
    else if (line == "exit_all")
    {
      loop_->shutdown();
    }
    else
    {
      conn->send(line);
      conn->send("\r\n");
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
