#include <libnet/timewheel.h>
#include <libnet/server.h>
#include <libnet/timer.h>
#include <libnet/eventloop.h>
#include <signal.h>

using namespace libnet;

class IdleServer
{
public:
  typedef std::shared_ptr<Connection> Conn;

public:
  IdleServer(EventLoop* loop, const char* host, int port, size_t seconds)
    : loop_(loop),
      server_(loop, host, port, 0),
      timewheel_(seconds),
      timerId_()
  {

  }

  void start()
  { 
    server_.setConnectionCallBack(std::bind(&IdleServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallBack(std::bind(&IdleServer::onMessage, this, std::placeholders::_1));
    server_.start();
    timerId_ = loop_->runInterval(1, 1000, std::bind(&IdleServer::onTimer, this));
  }

  void onConnection(const Conn& conn)
  {
    timewheel_.onConnection(conn);
  }

  void onMessage(const Conn& conn)
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

  void onTimer()
  {
    timewheel_.rotate();
  }

  ~IdleServer()
  {
    if (timerId_)
      loop_->cancel(timerId_);
  }

private:
  EventLoop* loop_;
  Server server_;
  TimeWheel timewheel_;
  TimerId timerId_;

};

EventLoop* gLoop;
void stop(int sig)
{
  if (gLoop)
    gLoop->shutdown();
};

int main(int argc, char *argv[])
{
    if (argc < 5)
  {
    LOG_ERROR << "<program> <ip> <port> <loglevel> <seconds>" ;
    exit(1);
  }
  char *host = static_cast<char *>(argv[1]);
  int port = atoi(argv[2]); 
  int level = atoi(argv[3]);
  log::LogLevel logLevel = log::LogLevel(level);
  int seconds = atoi(argv[4]);
  log::Logger::setLogLevel(logLevel);
  EventLoop loop;
  gLoop = &loop;
  ::signal(SIGINT, stop);
  IdleServer server(&loop, host, port, seconds);
  server.start();
  loop.loop();
};