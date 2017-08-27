#include <signal.h>
#include <vector>
#include <libnet/eventloop.h>
#include <libnet/eventloop_group.h>
#include <libnet/inet_address.h>
#include <libnet/countdown_latch.h>
#include <libnet/logger.h>
#include <libnet/mc/proxy_server.h>

using namespace std;
using namespace libnet;
using namespace mc::proxy;


std::function<void()> gStopCallback;
void stop(int sig)
{
  LOG_INFO << "stop" ;
  if (gStopCallback)
    gStopCallback();
};
// a memcached proxy server, 
int main(int argc, char *argv[])
{
  log::LogLevel logLevel = log::LogLevel(2);
  log::Logger::setLogLevel(logLevel);
  EventLoop loop;
  EventLoopGroup server_loops(&loop, 1, "server");
  EventLoopGroup client_loops(&loop, 1, "client");
  server_loops.start();
  client_loops.start();
  std::vector<InetAddress> remote_addr_list;
  remote_addr_list.push_back(InetAddress("0.0.0.0", 11211));
  remote_addr_list.push_back(InetAddress("0.0.0.0", 11212));
  CountDownLatch connected_latch(1);
  CountDownLatch closed_latch(0);
  MemcachedProxy proxy(&loop, 
                       &server_loops, 
                       &client_loops, 
                       InetAddress("0.0.0.0", 9000), 
                       remote_addr_list,
                       connected_latch,
                       closed_latch);
  proxy.start();
  gStopCallback = [&loop, &proxy, &closed_latch]()mutable{
    proxy.close();
    loop.runInterval(100, 1000, [&loop, &closed_latch]()mutable{
      if (closed_latch.count() == 0)
        loop.shutdown();
    });
  };
  connected_latch.wait();
  ::signal(SIGINT, stop);
  loop.loop();
}