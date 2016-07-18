#include <libnet/eventloop.h>
#include <libnet/server.h>
#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/logger.h>
#include <libnet/thread.h>
#include <functional>

using namespace libnet;
using namespace std::placeholders;

typedef std::shared_ptr<Connection> ConnectionPtr;

void onConnection(std::string name, const ConnectionPtr& conPtr)
{
  if (conPtr->connected())
  {
    LOG_INFO << name << " connection id=" << (conPtr->id()) << " connected";
  }
  else if (conPtr->disconnected())
  {
    LOG_INFO << name << "connection id=" << (conPtr->id()) << " disconnected";
  }
};

// void onMessage(ConnectionPtr conPtr)
// {

// };

void server_start()
{
  EventLoop loop;
  Server server(&loop, "127.0.0.1", 9999, 0);
  server.setConnectionCallBack(std::bind(onConnection, "server", _1));
  server.start();
  loop.loop();
};


void client_start()
{
  EventLoop loop;
  Client client(&loop, "127.0.0.1", 9999, 1);
  client.setConnectionCallBack(std::bind(onConnection, "client", _1));
  client.connect();
  loop.loop();
};

int main()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  Thread server(std::bind(server_start), "server");
  server.start();

  Thread client(std::bind(client_start), "client");
  client.start();

  //Connector connector()
  server.join();
  client.join();
  return 0;
}