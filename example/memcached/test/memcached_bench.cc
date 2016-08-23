#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include <memory>
#include <string.h>
#include "../memcached_client.h"
#include "../message.h"
#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

using namespace std;
using namespace libnet;

MemcachedClient* gClient;
EventLoop* gLoop;
AtomicInt32 gCounter(0);

struct MemcacheOpt
{
void set(int count, const std::string& value)
{
  int succ = 0;
  for (int i = 0; i < count; i++)
  {
    char buf[16];
    sprintf(buf, "key-%d", i);
    std::shared_ptr<Future> set = gClient->set(buf, 0, value);
    set->wait();
    LOG_DEBUG << "set " << i ;
    if (set->code() == kSucc) 
      succ++;
  }
  gCounter.addAndGet(succ);
}

void get(int count, const std::string& value)
{
  int succ = 0;
  for (int i = 0; i < count; i++)
  {
    char buf[16];
    sprintf(buf, "key-%d", i);
    std::shared_ptr<Future> get = gClient->get(buf);
    get->wait();

    if (get->code() == kSucc) 
      succ++;
  }
  gCounter.addAndGet(succ);
}
};

struct ThreadInitializer
{
void profile()
{
  ::ProfilerRegisterThread();
}
};

int main(int argc, char *argv[])
{
  if (argc < 7)
  {
    LOG_ERROR << "<program> <memcached ip> <memcached port> <loglevel> (set|get) <bytes> <clients> <number>" ;
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
  char *opt = static_cast<char *>(argv[4]);
  bool isSet = false;
  if (::strncmp(opt, "set", 3) == 0)
    isSet = true;

  int bytes = atoi(argv[5]);
  int clients = atoi(argv[6]);
  int number = atoi(argv[7]);

  int numberPerClient = number / clients;


  EventLoopThread loopThread("loop");
  loopThread.start();
  CountDownLatch latch(1);
  gLoop = loopThread.getLoop();
  MemcachedClient client(gLoop, host, port, latch);
 
  gClient = &client;
  client.connect();
  latch.wait();


  char buf[1024];
  for (int i = 0; i < bytes; i++)
  {
    buf[i] = '1';
  }
  std::string value(buf, bytes);

  std::vector<std::shared_ptr<Thread>> threads;
  threads.reserve(clients);

  MemcacheOpt mo;
  std::function<void()> func ;
  if (isSet)
    func = std::bind(&MemcacheOpt::set, &mo, numberPerClient, value);
  else
    func = std::bind(&MemcacheOpt::get, &mo, numberPerClient, value);

  for (int i = 0; i < clients; i++)
  {
    char buf[16];
    bzero(buf, sizeof(buf));
    sprintf(buf, "t-%d", i);

    std::shared_ptr<Thread> thread(new Thread(func, buf));
    threads.push_back(thread);
  }

  Timestamp start = Timestamp::now();
  //#ifdef PROFILER
  //ProfilerStart("memcached_bench.prof");
  //#endif
  for (int i = 0; i < clients; i++)
  {
     threads[i]->start();
  }

  for (int i = 0; i < clients; i++)
  {
    threads[i]->join();
  }
  Timestamp end = Timestamp::now();
  //#ifdef PROFILER
  //ProfilerStop();
  //#endif
  int64_t time = end.value() - start.value();
  LOG_INFO << "clients = "  << clients << " number = " << number  << " succ = " << (gCounter.getValue()) << " time = " << (time/1000) << "ms";
  
  //g_loop->runAfter(20000, std::bind(&EventLoop::shutdown, g_loop));
  return 0;
}


