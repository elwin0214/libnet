#ifndef __LIBNET_MEMCACHED_SERVER_H__
#define __LIBNET_MEMCACHED_SERVER_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include <libnet/mutexlock.h>
#include "message.h"

namespace memcached
{
namespace server
{

class MemcachedServer : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

  MemcachedServer(EventLoop* loop, const char* ip, int port);

  void start();
  
  void onMessage(const ConnectionPtr& connection);

  void onConnection(const ConnectionPtr& connection);
  
private:
  Server server_;
  MemcachedProcessor processor_;

};

}
}

#endif