#include <libnet/server.h>
#include <libnet/logger.h>
#include <libnet/eventloop_thread.h>
#include <libnet/mc/session.h>
#include <gtest/gtest.h>
#include <atomic>

using namespace std;
using namespace libnet;
using namespace mc::client;

typedef shared_ptr<Connection> Conn;

const char* gHost = "0.0.0.0";
uint16_t gPort = 9000;

TEST(Session, idle_close)
{ 
  log::Logger::setLogLevel(log::LogLevel(0));
  std::atomic<int> req_num(0);
  Server* server;
  EventLoop* loop;
  CountDownLatch server_latch(1);
  Thread([&server, &loop, &server_latch]()mutable{
    EventLoop tloop;
    Server tserver(&tloop, gHost, gPort);
    server = &tserver;
    loop = &tloop;
    server_latch.countDown();
    tserver.start();
    tloop.loop();
  }).start();
  server_latch.wait();

  server->setMessageCallBack([&req_num](const Conn& conn)mutable{
    Buffer& buf = conn->input();
    const char* pos = buf.find("\r\n");
    if (NULL == pos) return;
    buf.skip(pos - buf.beginRead() + 2);
    req_num.fetch_add(1);
  });

  EventLoopThread loop_thread("client");
  loop_thread.start();
  CountDownLatch connected_latch(1);
  CountDownLatch closed_latch(0);

  Session session(loop_thread.getLoop(), gHost, gPort, connected_latch, closed_latch, 4096, 1000, 3);
  session.connect();

  connected_latch.wait();
  closed_latch.wait();
  ASSERT_EQ(req_num.load(), 3) << " req_num is " << req_num.load();
  loop->shutdown();
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}