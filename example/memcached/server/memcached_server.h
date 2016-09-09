#ifndef __LIBNET_MEMCACHED_SERVER_H__
#define __LIBNET_MEMCACHED_SERVER_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/mutexlock.h>
#include "slab.h"
#include "processor.h"
#include "hashtable.h"
#include "lru.h"

namespace memcached
{
namespace server
{
using namespace std::placeholders;

class MemcachedServer : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

  MemcachedServer(EventLoop* loop, const char* ip, int port, size_t max_connections);

  MemcachedServer(EventLoop* loop, const char* ip, int port, size_t max_connections, const SlabOption& option);

  void start();
  
  void onMessage(const ConnectionPtr& connection);

  void onConnection(const ConnectionPtr& connection);

  size_t getNumConnections() { return num_connections_; }

private:

  void remove(Item* item);
  Item* find(const char* key, bool lru);
  Item* alloc(size_t item_size);
  void add(Item* item);
  void refreshTime() { now_ = Timestamp::now().secondsValue(); };
  uint64_t get_now() { return now_; }

private:
  Server server_;
  const size_t kMaxConnecitons_;
  size_t num_connections_;

  MemcachedProcessor processor_;
  HashTable hash_table_;
  SlabArray slab_array_;
  LRUList lru_list_;
  uint64_t now_;

};

}
}

#endif