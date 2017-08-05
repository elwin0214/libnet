#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include <memory>
#include <string.h>
#include <iostream>
#include <vector>
#include <libnet/mc/async_client.h>
#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

using namespace std;
using namespace libnet;
using namespace mc::client;

AsyncClient* gClient;
EventLoop* gLoop;
std::atomic<int> gCounter(0);
typedef shared_ptr<Message> Msg;

struct MemcacheOpt
{
void set(int count, const std::string& value)
{
  vector<future<Msg>> futures;
  futures.reserve(count);
  for (int i = 0; i < count; i++)
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "key-%d", i);
    LOG_TRACE << i << " " << buf ;
    auto f = gClient->set(buf, value, 0);
    futures.push_back(std::move(f)); 
  }
  int succ = 0;
  for(future<Msg>& f : futures)
  {
    f.wait();
    succ++;
  }
  gCounter.fetch_add(succ);
}

void get(int count, const std::string& value)
{
  vector<future<Msg>> futures;
  futures.reserve(count);
  for (int i = 0; i < count; i++)
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "key-%d", i);
    auto f = gClient->get(buf);
    futures.push_back(std::move(f)); 
  }
  int succ = 0;
  for(future<Msg>& f : futures)
  {
    f.wait();
    succ++;
  }
  gCounter.fetch_add(succ);
}
};

struct ThreadInitializer
{
void profile()
{
  #ifdef PROFILE
  ::ProfilerRegisterThread();
  #endif
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
  int reqs = atoi(argv[7]);

  //int numberPerClient = number / clients;


  EventLoopThread loopThread("loop");
  loopThread.start();
  CountDownLatch connected_latch(clients);
  CountDownLatch closed_latch(0);

  gLoop = loopThread.getLoop();
  AsyncClient client(gLoop, host, port, connected_latch, closed_latch, clients);
 
  gClient = &client;
  client.connect();
  connected_latch.wait();


  char buf[4096];
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
    func = std::bind(&MemcacheOpt::set, &mo, reqs/clients, value);
  else
    func = std::bind(&MemcacheOpt::get, &mo, reqs/clients, value);

  for (int i = 0; i < clients; i++)
  {
    char buf[16];
    bzero(buf, sizeof(buf));
    sprintf(buf, "t-%d", i);

    std::shared_ptr<Thread> thread(new Thread(func, buf));
    threads.push_back(thread);
  }

  Timestamp start = Timestamp::now();
  #ifdef PROFILE
  ::ProfilerStart("cpu.out");
  cout << "profile start" << endl;
  #endif
  for (int i = 0; i < clients; i++)
  {
    threads[i]->start();
  }

  for (int i = 0; i < clients; i++)
  {
    threads[i]->join();
  }
  Timestamp end = Timestamp::now();
  #ifdef PROFILE
  ::ProfilerStop();
  #endif
  int64_t time = end.value() - start.value();
  LOG_WARN << " opname = " << (isSet ? "set" : "get")
           << " clients = "  << clients 
           << " reqs = " << reqs 
           << " bytes = " << bytes 
           << " reqs = " << (gCounter.load()) 
           << " time = " << (time/1000) << "ms"
           << " qps = " << (reqs * 1.0 / (time * 1.0 / 1000 / 1000) );
  client.disconnect();
  closed_latch.wait();
  return 0;
}


