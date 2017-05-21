#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include "../memcached_server.h"

using namespace libnet;
using namespace memcached::server;

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    LOG_ERROR << "<program> <memcached ip> <memcached port> <loglevel>" ;
    exit(1);
  }

  char *host = static_cast<char *>(argv[1]);
  int port = atoi(argv[2]); 
  int level = atoi(argv[3]);
  log::LogLevel logLevel = log::LogLevel(level);
  setLogLevel(logLevel);
  EventLoop loop;
  MemcachedServer server(&loop, host, port, 1000);
  server.start();
  loop.loop();
  return 0;
}
