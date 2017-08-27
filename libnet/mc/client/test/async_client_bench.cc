#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_group.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include <memory>
#include <string.h>
#include <iostream>
#include <vector>
#include <stdio.h>
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
  int batch = 2000;
  int batch_num = count / batch;
  for (int b = 0; b < batch_num; b++)
  {
    vector<future<Msg>> futures;
    futures.reserve(batch);
    for (int i = 0; i < batch; i++)
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
      Msg msg = f.get();
      if (msg->code() == kSucc)
        succ++;
    }
    gCounter.fetch_add(succ);
  }
}

void get(int count, const std::string& value)
{
  int batch = 2000;
  int batch_num = count / batch;
  for (int b = 0; b < batch_num; b++)
  {
    vector<future<Msg>> futures;
    futures.reserve(batch);
    for (int i = 0; i < batch; i++)
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
      Msg msg = f.get();
      if (msg->code() == kSucc)
        succ++;
    }
    gCounter.fetch_add(succ);
  }
}
};

bool gProf = false;
struct ThreadInitializer
{
void profile()
{
  #ifdef PROFILE
  if (gProf)
    ::ProfilerRegisterThread();
  #endif
}
};

int main(int argc, char *argv[])
{
  if (argc < 8)
  {
    LOG_ERROR << "<program> <memcached ip> <memcached port> <loglevel> (set|get) <bytes> <threads> <clients> <reqs> <prof>" ;
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
  int thread_num = atoi(argv[6]);
  int clients = atoi(argv[7]);
  int reqs = atoi(argv[8]);
  gProf = (1 == atoi(argv[9]));
  //int numberPerClient = number / clients;

  CountDownLatch connected_latch(clients);
  CountDownLatch closed_latch(0);
  EventLoop loop;
  EventLoopGroup loop_group(&loop, thread_num, "io");
  loop_group.start();
  AsyncClient client(&loop_group, host, port, connected_latch, closed_latch, clients);
 
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
  if (gProf)
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
  if (gProf)
    ::ProfilerStop();
  #endif
  int64_t time = end.value() - start.value();
  double qps = (reqs * 1.0 / (time * 1.0 / 1000 / 1000));
  LOG_WARN << " opname = " << (isSet ? "set" : "get")
           << " clients = "  << clients 
           << " reqs = " << reqs 
           << " bytes = " << bytes 
           << " succ = " << (gCounter.load()) 
           << " time = " << (time/1000) << "ms"
           << " qps = " << qps;

  fprintf(stderr, "opname = %s clients = %d bytes =%d reqs = %d succ = %d time = %dms qps = %.2f\n",
                  (isSet ? "set" : "get"), clients, bytes, reqs, gCounter.load(), time/1000, qps);
  client.disconnect();
  closed_latch.wait();
  return 0;
}


