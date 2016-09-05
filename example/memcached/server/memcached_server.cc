#include <memory>
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/buffer.h>
#include <libnet/logger.h>
#include "memcached_server.h"
#include "memcached_context.h"

namespace memcached
{
namespace server
{
using namespace libnet;

MemcachedServer::MemcachedServer(EventLoop* loop, const char* ip, int port, size_t max_connections)
  : server_(loop, ip, port, 1), //single thread server
    kMaxConnecitons_(max_connections),
    num_connections_(0),
    processor_(),
    hash_table_(32, 1.2),
    slab_array_(SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 64)) //64M
{

};

MemcachedServer::MemcachedServer(EventLoop* loop, const char* ip, int port, size_t max_connections, const SlabOption& option)
  : server_(loop, ip, port, 1), //single thread server
    kMaxConnecitons_(max_connections),
    num_connections_(0),
    processor_(),
    hash_table_(32, 1.2),
    slab_array_(option) 
{

};

void MemcachedServer::start()
{
  slab_array_.init();
  processor_.set_item_find(std::bind(&HashTable::get, &hash_table_, _1));
  processor_.set_item_alloc(std::bind(&SlabArray::pop, &slab_array_, _1));
  processor_.set_item_add(std::bind(&HashTable::addItem, &hash_table_, _1));
  processor_.set_item_remove(std::bind(&MemcachedServer::remove, this, _1));
  processor_.init();
  server_.setConnectionCallBack(std::bind(&MemcachedServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&MemcachedServer::onMessage, this, std::placeholders::_1));
  server_.start();
};

void MemcachedServer::onConnection(const ConnectionPtr& connection)
{
  if (connection->connected())
  {
    num_connections_++;
    if (num_connections_ > kMaxConnecitons_)
    {
      connection->shutdown();
      return;
    }
    std::shared_ptr<MemcachedContext> ctx = std::make_shared<MemcachedContext>();
    connection->setContext(ctx);
    ctx->set_send_func(std::bind(&Connection::send, connection.get(), std::placeholders::_1));
    ctx->set_close_func(std::bind(&Connection::shutdown, connection.get()));
  }
  else
  {
    num_connections_-- ;
  }
};

void MemcachedServer::onMessage(const ConnectionPtr& connection)
{
  std::shared_ptr<MemcachedContext> context = std::static_pointer_cast<MemcachedContext>(connection->getContext());
  Buffer& buffer = connection->input();
  LOG_TRACE << buffer.toString() ;
  processor_.process(buffer, *context);
};
  
}
}