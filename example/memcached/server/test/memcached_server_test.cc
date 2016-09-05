#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include "../memcached_server.h"

using namespace libnet;
using namespace memcached::server;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
      LOG_ERROR << "<program> <loglevel>" ;
      exit(1);
  }
  int level = atoi(argv[1]); 
  log::LogLevel logLevel = log::LogLevel(level);
  setLogLevel(logLevel);
  EventLoop loop;
  MemcachedServer server(&loop, "127.0.0.1", 11211, 2);
  server.start();
  loop.loop();
  return 0;
}
