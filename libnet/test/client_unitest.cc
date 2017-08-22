#include <libnet/logger.h>
#include <libnet/client.h>
#include <libnet/server.h>
#include <libnet/countdown_latch.h>
#include <libnet/eventloop_thread.h>
#include <libnet/eventloop_group.h>
#include <libnet/connection.h>
#include <gtest/gtest.h>
#include <atomic>

using namespace libnet;
using namespace std;
typedef std::shared_ptr<Connection> Conn;
const char* gHost = "0.0.0.0";
uint16_t gPort = 18081;

TEST(client, retry_one_thread)
{
  int connected = 0;
  int server_close = 0;
  int client_close = 0;

  int retry = 3;
  EventLoop loop;
  Server server(&loop, gHost, gPort);
  server.setConnectionCallBack([&server_close](const Conn& conn)mutable{
    if (conn->connected())
    {
      conn->shutdown();
    }
    else
    {
      server_close++;
    }
  });

  server.start();

  Client client(&loop, gHost, gPort);
  client.setConnectionCallBack([&connected, &client_close, &client, &retry](const Conn& conn)mutable{
    if (conn->connected())
    {
      connected++;
      if (connected >= retry)
        client.disableRetry();
    }
    else
    {
      client_close++;
    }
  });
  client.enableRetry();
  client.connect();
  loop.runInterval(10, 10, [&loop, &client_close, &server_close, &retry]()mutable{
    if (client_close == retry && server_close == retry)//Server & Client will not hold connection
      loop.shutdown();
  });
  loop.loop();
}

TEST(client, multi_client_online)
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  std::atomic<int> connected(0);
  std::atomic<int> client_close(0);
  std::atomic<int> server_close(0);

  int number = 100;
  bool stop = false;
  EventLoop loop;
  EventLoopGroup server_loops(&loop, 2, "server");
  EventLoopGroup client_loops(&loop, 2, "client");

  Server server(&loop, gHost, gPort, &server_loops);
  server.setConnectionCallBack([&loop, &server_close](const Conn& conn)mutable{
    if (!(conn->connected()))
    {
      server_close.fetch_add(1);
    }
  });
  server_loops.start();
  server.start();

  client_loops.start();
  vector<shared_ptr<Client>> clients;
  clients.reserve(number);
  for (int i = 0; i < number; i++)
  {
    auto c = make_shared<Client>(client_loops.getNextLoop(), gHost, gPort);
    clients.push_back(c);
    c->enableRetry();
    c->setConnectionCallBack([&connected, &client_close](const Conn& conn){
      if (conn->connected())
        connected.fetch_add(1);
      else
        client_close.fetch_add(1);
    });
    c->connect();
  }

  loop.runInterval(10, 100, [&loop, &connected, &number, &clients]()mutable{
    if (connected >= number)
    {
      for (auto& c : clients)
        c->disconnect();
    }
  });

  loop.runInterval(10, 100, [&loop, &number, &server_close, &client_close]()mutable{
    if (client_close >= number && server_close >= number)// not 100% safe
      loop.shutdown();
  });
  loop.loop();
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

