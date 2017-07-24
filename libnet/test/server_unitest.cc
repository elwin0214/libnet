#include <libnet/logger.h>
#include <libnet/client.h>
#include <libnet/server.h>
#include <libnet/countdown_latch.h>
#include <libnet/eventloop_thread.h>
#include <libnet/eventloop_group.h>
#include <libnet/connection.h>
#include <gtest/gtest.h>

using namespace libnet;
using namespace std;

typedef shared_ptr<Connection> Conn;

const char* gHost = "0.0.0.0";
uint16_t gPort = 18081;
const char* gMessage = "HELLO\r\n";

TEST(server, hello_one_thread)
{
  bool connected = false;
  bool request = false;
  bool response = false;
  EventLoop loop;
  Server server(&loop, gHost, gPort);
  server.setMessageCallBack([&loop, &request](const Conn& conn)mutable{
    auto& buf = conn->input();
    const char* pos = buf.find("\r\n");
    if (NULL == pos) return;
    size_t len = pos - buf.beginRead() + 2;
    string str = string(buf.beginRead(), len);
    ASSERT_EQ(str, gMessage);
    request = true;
    buf.skip(len);
    conn->send(gMessage);
    conn->shutdown();
  });
  server.start();
  EventLoopThread thread("client");
  thread.start();
  Client client(thread.getLoop(), gHost, gPort);
  client.setConnectionCallBack([&connected](const Conn& conn)mutable{
    if (conn->connected())
    {
      connected = true;
      conn->send(gMessage);
    }
  });
  client.setMessageCallBack([&response](const Conn& conn)mutable{
    auto& buf = conn->input();
    const char* pos = buf.find("\r\n");
    if (NULL == pos) return;
    size_t len = pos - buf.beginRead() + 2;
    string str = string(buf.beginRead(), len);
    ASSERT_EQ(str, gMessage);
    response = true;
    buf.skip(len);
  });
  client.connect();
  loop.runAfter(1000, [&loop, &connected, &request, &response]()mutable{
    ASSERT_TRUE(connected && request && response);
    loop.shutdown();
  });
  loop.loop();
}

TEST(server, hello_multi_thread)
{
  int number = 100;
  
  std::atomic<int> connected(0);
  std::atomic<int> request(0);
  std::atomic<int> response(0);

  EventLoop loop;
  EventLoopGroup server_loops(&loop, 2, "server");
  Server server(&loop, gHost, gPort, &server_loops);
  server.setMessageCallBack([&loop, &request](const Conn& conn)mutable{
    auto& buf = conn->input();
    const char* pos = buf.find("\r\n");
    if (NULL == pos) return;
    size_t len = pos - buf.beginRead() + 2;
    string str = string(buf.beginRead(), len);
    ASSERT_EQ(str, gMessage);
    request.fetch_add(1);
    buf.skip(len);
    conn->send(gMessage);
    conn->shutdown();
  });
  server_loops.start();
  server.start();

  EventLoopGroup client_loops(&loop, 2, "client");
  client_loops.start();
  vector<shared_ptr<Client>> clients;
  clients.reserve(100);
  
  for (int i = 0; i < number; i++)
  {
    auto c = make_shared<Client>(client_loops.getNextLoop(), gHost, gPort);
    clients.push_back(c);
    c->setConnectionCallBack([&connected](const Conn& conn)mutable{
      if (conn->connected())
      {
        connected.fetch_add(1);
        conn->send(gMessage);
      }
    });
    c->setMessageCallBack([&response](const Conn& conn)mutable{
      auto& buf = conn->input();
      const char* pos = buf.find("\r\n");
      if (NULL == pos) return;
      size_t len = pos - buf.beginRead() + 2;
      string str = string(buf.beginRead(), len);
      ASSERT_EQ(str, gMessage);
      response.fetch_add(1);
      buf.skip(len);
    });
    c->connect();
  }
  
  loop.runInterval(1000, 1000, [&loop, &connected, &request, &response, &number]()mutable{
    if (connected == number && request == number && response == number) // not 100% safe
    {
      loop.shutdown();
    }    
  });
  loop.loop();
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


