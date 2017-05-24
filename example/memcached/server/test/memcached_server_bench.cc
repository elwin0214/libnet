#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/thread.h>
#include <signal.h>
#include <functional>
#include "../memcached_server.h"
#include "../memcache.h"

#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

using namespace libnet;
using namespace memcached::server;

struct ThreadInitializer
{
void profile()
{
  #ifdef PROFILE
  ::ProfilerRegisterThread();
  #endif
}
};

EventLoop* gLoop;
void stop(int signo)   
{ 
  LOG_ERROR << "stop";
  gLoop->shutdown();
  #ifdef PROFILE
  ::ProfilerStop();
  #endif
}  

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



  #ifdef PROFILE
  ThreadInitializer initialzer;
  Thread::registerInitCallback(std::bind(&ThreadInitializer::profile, &initialzer));
  #endif

  EventLoop loop;
  gLoop = &loop;
  MemcachedServer server(&loop, host, port, 1000);
  //MemCache cache;
  MemCache cache(24, 1.2, SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 1024));

  #ifdef PROFILE
  ProfilerStart("cpu.out");
  #endif

  server.set_item_find(std::bind(&MemCache::find, &cache, _1, _2));
  server.set_item_alloc(std::bind(&MemCache::alloc, &cache, _1));
  server.set_item_add(std::bind(&MemCache::add, &cache, _1));
  server.set_item_remove(std::bind(&MemCache::remove, &cache, _1));

  server.start();
  ::signal(SIGINT, stop);
  loop.loop();
  return 0;
}
