#ifndef __LIBNET_MC_PROXY_SERVER_H__
#define __LIBNET_MC_PROXY_SERVER_H__
#include <memory>
#include <vector>
#include <map>
#include <libnet/countdown_latch.h>

namespace libnet
{
class Connection;
class InetAddress;
class EventLoop;
class EventLoopGroup;
}

namespace mc
{
namespace client
{
class Command;
}
}

namespace mc
{
namespace proxy
{
class FrontServer;
class BackClient;
using namespace libnet;
using namespace mc::client;

class MemcachedProxy : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> Conn;
  typedef std::shared_ptr<Command> Cmd;

public:
  MemcachedProxy(EventLoop* loop,
                 EventLoopGroup* server_loops,
                 EventLoopGroup* client_loops,
                 const InetAddress& local_addr,
                 const std::vector<InetAddress>& remote_addr_list,
                 CountDownLatch& connected_latch,
                 CountDownLatch& closed_latch);

  void start();

  void close();

private:
  std::shared_ptr<FrontServer> server_;
  std::shared_ptr<BackClient> client_;
};

}
}

#endif