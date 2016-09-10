#include "idle_server.h"

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
  IdleServer server(&loop, host, port, seconds);
  server.start();
  loop.loop();
};