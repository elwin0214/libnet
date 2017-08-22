#include <functional>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <libnet/logger.h>
#include <libnet/connection.h>
#include <libnet/cstring.h>
#include <libnet/server.h>
#include <signal.h>

using namespace libnet;

class EchoServer
{
public:
  EchoServer(EventLoop* loop, const char* host, uint16_t port)
    : loop_(loop),
      server_(loop, host, port)
  {
  }

  void start()
  { 
    server_.setMessageCallBack(std::bind(&EchoServer::onMessage, this, std::placeholders::_1));
    server_.start();
  };

  void onMessage(const std::shared_ptr<Connection>& conn)
  {
    Buffer& buffer = conn->input();
    const char* pos = buffer.find("\r\n");
    if (NULL == pos) return;
    size_t len = pos - buffer.beginRead() + 2;
    std::string str = std::string(buffer.beginRead(), len);
    buffer.skip(len);
    if (str == "exit\r\n")
    {
      conn->shutdown();
      return;
    }
    else if (str == "exit all\r\n")
    {
      loop_->shutdown();
      return;
    }
    
    conn->send(str);
  };

private:
  EventLoop* loop_;
  Server server_; 
};

EventLoop* gLoop;
void stop(int sig)
{
  if (gLoop)
    gLoop->shutdown();
};

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    LOG_ERROR << "<program> <ip> <port> <loglevel>" ;
    exit(1);
  }
  char *host = static_cast<char *>(argv[1]);
  int port = atoi(argv[2]); 
  int level = atoi(argv[3]);
  log::LogLevel logLevel = log::LogLevel(level);
  log::Logger::setLogLevel(logLevel);
  EventLoop loop;
  gLoop = &loop;
  ::signal(SIGINT, stop);
  EchoServer server(&loop, host, port);
  server.start();
  loop.loop();
  return 0;
}