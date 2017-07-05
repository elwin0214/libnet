
#include <libnet/countdown_latch.h>
#include <libnet/client.h>
#include <libnet/eventloop_group.h>
#include <libnet/connection.h>
#include <libnet/thread.h>
#include <memory>
#include <string.h>
#include <iostream>
#include <vector>
#include <memory>
#include "../../message/mcode.h"
#include "../../message/response.h"
#include "../../message/response_codec.h"

#ifdef PROFILE
#include <gperftools/profiler.h>
#endif

using namespace std;
using namespace libnet;
using namespace memcached::message;


struct ThreadInitializer
{
void profile()
{
  #ifdef PROFILE
  ::ProfilerRegisterThread();
  #endif
}
};

class MemClient
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

  MemClient(EventLoop* loop, 
            const char* host, 
            int port,
            CountDownLatch& connected,
            CountDownLatch& finished,
            Opt op,
            int bytes,
            int index,
            int sum)
    : client_(loop, host, port),
      connected_(connected),
      finished_(finished),
      respc_(),
      op_(op),
      bytes_(bytes),
      value_(string(bytes, 'a')),
      response_(new Response(op)),
      index_(index),
      req_(0),
      sum_(sum)
  {
    client_.setConnectionCallBack(std::bind(&MemClient::onConnection, this, std::placeholders::_1));
    client_.setMessageCallBack(std::bind(&MemClient::onMessage, this, std::placeholders::_1));
    client_.connect();
  }

  ~MemClient()
  {
    LOG_TRACE << "~MemClient() "  << conn_.use_count();
  }

  void onConnection(const ConnectionPtr& conn)
  {
    if (conn->connected())
    {
      conn_ = conn;
      connected_.countDown();
    }
    else
    {
      LOG_TRACE << "onConnection() p1 "  << conn_.use_count();
      conn_.reset();
      LOG_TRACE << "onConnection() p2 "  << conn_.use_count();
      finished_.countDown(); //a handleClose调用
    }

  }

  void onMessage(const ConnectionPtr& conn)
  {
    receive();
  }

  void send()
  {
    if (op_ == Opt::kSet)
    {
      Buffer buffer(0, 2048);
      char buf[128];
      ::bzero(buf, sizeof(buf));
      snprintf(buf, sizeof(buf), "set k-%d-%d 2 0 %d\r\n", index_, req_, bytes_);
      buffer.append(buf);
      buffer.append(value_);
      buffer.append("\r\n");
      conn_->sendBuffer(&buffer);
    }
    else
    {
      char buf[128];
      ::bzero(buf, sizeof(buf));
      snprintf(buf, sizeof(buf), "get k-%d-%d\r\n", index_, req_);
      conn_->send(buf);
    }
  }
  
  void receive()
  {
    if (respc_.decode(*response_, conn_->input()))
    {
      if(++req_ <= sum_)
        send();
      else
      {
        conn_->shutdown(); 
        // 如果在这里 finished_.countDown(); Client可能先析构，调用conn_析构函数
        //server端关闭连接后，handleClose里面会再调用一次conn_.reset(),并发操作shared_ptr。
      }
    }
  }

private:
  Client client_;
  ConnectionPtr conn_;
  CountDownLatch& connected_;
  CountDownLatch& finished_;
  ResponseCodec respc_;
  Opt op_;
  int bytes_;
  string value_;
  unique_ptr<Response> response_;
  int index_;
  int req_;
  int sum_;
};


int main(int argc, char *argv[])
{
  if (argc < 7)
  {
    LOG_ERROR << "<program> <memcached ip> <memcached port> <loglevel> (set|get) <bytes> <clients> <threads> <reqs>" ;
    exit(1);
  }

  char *host = static_cast<char *>(argv[1]);
  int port = atoi(argv[2]); 
  int level = atoi(argv[3]);
  char *opt = static_cast<char *>(argv[4]);
  Opt op = kSet;
  if (string("get") == opt)
  {
    op = Opt::kGet;
  }
  int bytes = atoi(argv[5]);
  int clients = atoi(argv[6]);
  int threads = atoi(argv[7]);
  int reqs = atoi(argv[8]);

  log::LogLevel logLevel = log::LogLevel(level);
  setLogLevel(logLevel);
  #ifdef PROFILE
  ThreadInitializer initialzer;
  Thread::registerInitCallback(std::bind(&ThreadInitializer::profile, &initialzer));
  #endif

  #ifdef PROFILE
  ::ProfilerStart("cpu.out");
  cout << "profile start" << endl;
  #endif

  EventLoop loop;
  EventLoopGroup loops(&loop, threads, "bench");
  CountDownLatch connected(clients);
  CountDownLatch finished(clients);
  loops.start();
  vector<shared_ptr<MemClient>> mclients;
  for (int i = 0; i < clients; i++)
  {
    shared_ptr<MemClient> sp(new MemClient(loops.getNextLoop(), host, port, connected, finished, op, bytes, i, reqs/clients));
    mclients.push_back(sp);
  }
  connected.wait();
  Timestamp start = Timestamp::now();

  for (int i = 0; i < clients; i++)
  {
    mclients[i]->send();
  }
  finished.wait();
  // ~Client() 与 client handleClose(server 关闭连接) 两个线程都会触发对 Connection的调用，保证执行到这里时候，即使持有conn的shared_ptr 调用Connection的函数，也不会访问析构过的内存。
  Timestamp end = Timestamp::now();
  int64_t time = end.value() - start.value();
  LOG_INFO << "clients = "  << clients << " reqs = " << reqs << " bytes = " << bytes << " time = " << (time/1000) << "ms";
  
  #ifdef PROFILE
  ::ProfilerStop();
  #endif
  return 0;
}