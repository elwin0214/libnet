#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include <string>
#include <libnet/mc/async_client.h>
#include <gtest/gtest.h>

using namespace libnet;
using namespace mc::client;

EventLoop* gLoop; 
AsyncClient* gClient;
typedef std::shared_ptr<Message> Msg;

void test_set()
{
  future<Msg> f1 = gClient->set("a", "cc", 2000);
  future<Msg> f2 = gClient->set("b", "cc", 2000);
  future<Msg> f3 = gClient->get("b");

  f1.wait();
  f2.wait();
  f3.wait();
  Msg msg1 = f1.get();
  Msg msg2 = f2.get();
  Msg msg3 = f3.get();

  ASSERT_TRUE(msg1->code() == kSucc) << msg1->code_name();
  ASSERT_TRUE(msg2->code() == kSucc) << msg2->code_name();
  ASSERT_TRUE(msg3->code() == kSucc) << msg3->code_name();
  ASSERT_TRUE(msg3->data_.value_ == "cc");
}


void test_rem()
{
  future<Msg> f1 = gClient->remove("a");
  future<Msg> f2 = gClient->set("a", "cc", 2000);
  future<Msg> f3 =gClient->remove("a");

  f1.wait();
  f2.wait();
  Msg msg1 = f1.get();
  Msg msg2 = f2.get();
  ASSERT_TRUE(msg2->code() == kSucc);
  f3.wait();
  Msg msg3 = f3.get();
  ASSERT_TRUE(msg3->code() == kSucc) << msg3->code_name();
}


void test_add()
{
  future<Msg> f1 = gClient->remove("a");
  f1.wait();
  future<Msg> f2 = gClient->add("a", "1", 2000);
  f2.wait();
  future<Msg> f3 = gClient->add("a", "99", 2000);
  f3.wait();
  Msg msg3 = f3.get();
  ASSERT_TRUE(msg3->code() == kFail);

  uint32_t last_incr;
  for (int i = 0 ; i < 100 ; i++)
  { 
    future<Msg> f = gClient->incr("a", 1);
    f.wait();
    Msg msg = f.get();
    ASSERT_TRUE(msg->code() == kSucc);
    last_incr = msg->data_.count_;
  }
  ASSERT_TRUE( last_incr == 101);


  for (int i = 0 ; i < 100 ; i++)
  {
    future<Msg> f = gClient->decr("a", 1);
    f.wait();
    Msg msg = f.get();
    ASSERT_TRUE(msg->code() == kSucc);
    last_incr = msg->data_.count_;
  }
  ASSERT_TRUE( last_incr == 1);
}
// 存在2个线程Loop 和 main，Client先于Loop析构
// 要考虑2个问题：1. 并发问题，shared_ptr 并发写（析构与reset）
// 2.Loop线程访问Client的资源,Client析构时候，注意Loop访问Client控制的资源被析构。
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

    EventLoopThread thread("loop");
    thread.start();
    CountDownLatch connected_latch(1);
    CountDownLatch closed_latch(0);

    gLoop = thread.getLoop();
    AsyncClient client(gLoop, host, port, connected_latch, closed_latch, 1);
    gClient = &client;
    client.connect();
    connected_latch.wait();
 
    test_set();
    test_rem();
    test_add();
    client.disconnect();
    closed_latch.wait();
    return 0;
}