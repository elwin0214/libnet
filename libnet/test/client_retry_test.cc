#include <libnet/logger.h>
#include <libnet/client.h>
#include <libnet/server.h>
#include <libnet/countdown_latch.h>
#include <libnet/eventloop_thread.h>
#include <libnet/connection.h>

using namespace libnet;
typedef std::shared_ptr<Connection> ConnectionPtr;
int gCount = 3;
EventLoop* gLoop;
int main()
{
  EventLoop loop;
  gLoop = &loop;

  EventLoopThread client_thread("client");
  EventLoop* client_loop = client_thread.start();
  Client client(client_loop, "127.0.0.1", 9999);

  Server server(gLoop, "127.0.0.1", 9999, 1);
  server.setConnectionCallBack([client_loop,&client](const ConnectionPtr& connection)
  {
    
    if (connection->connected())
    {
      LOG_INFO << "server :count = " << (gCount) << " connection = "<< (connection->get_name()) << " go to close!";
      //connection->shutdown();
      gLoop->runAfter(2000, std::bind(&Connection::shutdown, connection));
    }
    else
    {
      LOG_INFO << "server : connection = " << (connection->get_name()) << " closed!";
    }
    
  });

  server.start();
  LOG_INFO << "server start...";

  client.enableRetry();
  client.setConnectionCallBack([client_loop,&client](const ConnectionPtr& connection)
  {
    if (connection->connected())
    {
      LOG_INFO << "client : connection = "<< (connection->get_name()) << " connected!";

    }
    else
    {
      LOG_INFO << "client : connection = " << (connection->get_name()) << " closed!";
      gCount--;
      if (gCount == 0)
      {
        LOG_INFO << "server : goto close server!";
        gLoop->shutdown();
        client_loop->shutdown();
        client.disconnect();
      }
    }
  });
  client.connect();
  gLoop->loop();

  return 0;
}