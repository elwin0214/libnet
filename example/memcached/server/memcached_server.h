#ifndef __LIBNET_MEMCACHED_SERVER_H__
#define __LIBNET_MEMCACHED_SERVER_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/mutexlock.h>
#include <memory>
#include "../message/request_codec.h"
#include "../message/response_codec.h"

namespace memcached
{
namespace server
{
using namespace std::placeholders;
using namespace libnet;
using namespace memcached::message;
using namespace std;

class MemcachedServer : public libnet::NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

  MemcachedServer(EventLoop* loop, const char* ip, int port, size_t max_connections);

  void start();
  
  void onMessage(const ConnectionPtr& connection);

  void onConnection(const ConnectionPtr& connection);

  size_t getNumConnections() { return num_connections_; }

  void set_handler(function<void(Request&, Response&)> func) { handler_ = func; }


private:
  Server server_;
  const size_t kMaxConnecitons_;
  size_t num_connections_;
  function<void(Request&, Response&)> handler_;
  RequestCodec requestCodec_;
  ResponseCodec responseCodec_;

};

}
}

#endif