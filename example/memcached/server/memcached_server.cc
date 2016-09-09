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
    slab_array_(SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 64)), //64M
    lru_list_(slab_array_.slabs()),
    now_(Timestamp::now().secondsValue())
{

};

MemcachedServer::MemcachedServer(EventLoop* loop, const char* ip, int port, size_t max_connections, const SlabOption& option)
  : server_(loop, ip, port, 1), //single thread server
    kMaxConnecitons_(max_connections),
    num_connections_(0),
    processor_(),
    hash_table_(32, 1.2),
    slab_array_(option),
    lru_list_(slab_array_.slabs())
{

};

void MemcachedServer::start()
{
  //slab_array_.init();
  processor_.set_item_find(std::bind(&MemcachedServer::find, this, _1, _2));
  processor_.set_item_alloc(std::bind(&MemcachedServer::alloc, this, _1));
  processor_.set_item_add(std::bind(&MemcachedServer::add, this, _1));
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
  LOG_TRACE << "request = " << buffer.toString() ;
  processor_.process(buffer, *context);
};

void MemcachedServer::remove(Item* item)
{
  hash_table_.removeItem(item);
  slab_array_.push(item);
};

Item* MemcachedServer::find(const char* key, bool lru)
{
  Item* item = hash_table_.get(key);
  uint64_t now = Timestamp::now().secondsValue();
  if (NULL == item) return NULL;
  if (item->expired(now))
  {
    LOG_TRACE << "key = " << (item->key()) << " exptime = " << (item->get_exptime()) << " result = expired" ;
    lru_list_.remove(item);
    hash_table_.removeItem(item);
    slab_array_.push(item);
    return NULL;
  }
  if (lru)
  {
    lru_list_.access(item);
  }
  return item;
};

Item* MemcachedServer::alloc(size_t item_size)
{
  int index = -1; 
  uint64_t now = Timestamp::now().secondsValue();
  Item* item = slab_array_.pop(item_size, index);
  if (item != NULL) 
    return item;

  if (index >= 0)
  {
    assert(index < 256);
    item = lru_list_.recycle(index, now);
    if (NULL != item) 
      hash_table_.removeItem(item);
    item->set_time(0);
    item->set_exptime(0);
  }
  return item;
};

void MemcachedServer::add(Item* item)
{
  hash_table_.addItem(item);
  lru_list_.add(item);
};

}
}