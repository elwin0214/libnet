#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_group.h>
#include <libnet/thread.h>
#include <signal.h>
#include <functional>
#include <libnet/mc/mem_server.h>
#include <libnet/mc/mem_cache.h>
#include <libnet/mc/mem_handler.h>

#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

using namespace std;
using namespace libnet;
using namespace mc::server;

bool gProf = false;
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
}  

int main(int argc, char *argv[])
{
  if (argc < 6)
  {
    LOG_ERROR << "<program> <memcached ip> <memcached port> <threads> <loglevel> <prof>" ;
    exit(1);
  }

  char *host = static_cast<char *>(argv[1]);
  int port = atoi(argv[2]); 
  int threads = atoi(argv[3]);
  int level = atoi(argv[4]);
  gProf = 1 == atoi(argv[5]);
  log::LogLevel logLevel = log::LogLevel(level);
  setLogLevel(logLevel);

  #ifdef PROFILE
  ThreadInitializer initialzer;
  if (gProf)
    Thread::registerInitCallback(std::bind(&ThreadInitializer::profile, &initialzer));
  #endif

  std::map<string,string> map;
  EventLoop loop;
  EventLoopGroup loop_group(&loop, threads, "io");
  loop_group.start();
  gLoop = &loop;
  MemServer server(&loop, host, port, &loop_group, 1000);

  MemHandler handler(4, 
                     std::hash<std::string>(),
                     24, 
                     1.2, 
                     SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 1024));

  #ifdef PROFILE
  if (gProf)
   ProfilerStart("cpu.out");
  #endif
  auto func = std::bind(&MemHandler::handle, &handler, _1, _2);
  server.set_handler([&func, &map](Message& request, Message& response){

    func(request, response);

  });

  server.start();
  ::signal(SIGINT, stop);
  loop.loop();
  #ifdef PROFILE
  if (gProf)
   ::ProfilerStop();
  #endif
  return 0;
}
