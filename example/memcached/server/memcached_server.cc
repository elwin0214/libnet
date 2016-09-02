#include <memory>
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include "memcached_server.h"
#include "memcached_context.h"

namespace memcached
{
namespace server
{

MemcachedServer::MemcachedServer(EventLoop* loop, const char* ip, int port)
  : server_(loop, ip, port, 1), //single thread server
    processor_()
{
};

void MemcachedServer::onConnection(const ConnectionPtr& connection)
{
  if (connection->connected())
  {
    connection->setContext(std::make_shared<MemcachedContext>());
  }
};

void MemcachedServer::onMessage(const ConnectionPtr& connection)
{

  std::shared_ptr<MemcachedContext> context = std::static_pointer_cast<MemcachedContext>(connection->getContext());
  processor_.process(connection, connection->input(), context);
};
  
}
}