#include <libnet/nocopyable.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_group.h>
#include <libnet/connection.h>
#include <libnet/inet_address.h>
#include <libnet/client.h>
#include <libnet/server.h>
#include <libnet/mutexlock.h>
#include <string>
#include <future>
#include <libnet/mc/command.h>
#include "back_client.h"
#include "front_server.h"
#include <libnet/mc/proxy_server.h>

namespace mc
{
namespace proxy
{

using namespace libnet;
using namespace std;
typedef std::shared_ptr<Message> Msg;

MemcachedProxy::MemcachedProxy(EventLoop* loop,
                               EventLoopGroup* server_loops,
                               EventLoopGroup* client_loops,
                               const InetAddress& local_addr, 
                               const vector<InetAddress>& remote_addr_list,
                               CountDownLatch& connected_latch,
                               CountDownLatch& closed_latch)
  : server_(make_shared<FrontServer>(loop, server_loops, local_addr)),
    client_(make_shared<BackClient>(client_loops,remote_addr_list, connected_latch, closed_latch))
{
  FrontServer* server = server_.get();
  BackClient* client = client_.get(); // todo weak_ptr

  server_->setMessageCallBack([client, server](const Conn& cn, Message& request){
    std::weak_ptr<Connection> weak_cn = cn;
    auto response_callback = [weak_cn, server](const Msg& response){
      Conn cn = weak_cn.lock();
      if (cn)
        server->send(cn, *response); 
    };
    auto cmd = make_shared<Command>(request, response_callback);
    client->send(cmd);
  });
}

void MemcachedProxy::start()
{
  server_->start();
  client_->enableRetry();
  client_->connect();
}

void MemcachedProxy::close()
{
  client_->disconnect();
}

}
}
