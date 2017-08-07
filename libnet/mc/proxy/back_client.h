#ifndef __LIBNET_MEMCACHED_PROXY_BACK_H__
#define __LIBNET_MEMCACHED_PROXY_BACK_H__

#include <functional>
#include <vector>
#include <memory>
#include <libnet/nocopyable.h>
#include <libnet/countdown_latch.h>

namespace libnet 
{ 
class Connection; 
}

namespace mc
{
namespace client
{
class Session; 
class Command;
}
}

namespace mc
{
namespace proxy
{
using namespace std;
using namespace libnet;
using namespace mc::client;

class BackClient : public NoCopyable
{
public:
  typedef shared_ptr<Connection> Conn;
  typedef shared_ptr<Command> Cmd;

public:
  BackClient(EventLoopGroup* loop_group, 
             const std::vector<InetAddress>& remote_addresses,
             CountDownLatch& connected_latch,
             CountDownLatch& closed_latch);

  void connect();

  void enableRetry();

  void disconnect();

  void send(const Cmd& cmd);

private:
  vector<shared_ptr<Session>> sessions_;
  size_t size_;
  function<size_t(const string&)> hash_;
};

}
}

#endif