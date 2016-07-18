#ifndef __LIBNET_MEMCACHED_CLIENT_H__
#define __LIBNET_MEMCACHED_CLIENT_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include "message.h"

using namespace libnet;

class MemcachedClient : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;
  MemcachedClient(EventLoop* loop, const char* host, int port, CountDownLatch& countDownLatch)
    : client_(loop, host, port, 1),
      countDownLatch_(countDownLatch)
  {
  };

  void connect();

  void onConnection(const ConnectionPtr& conn);

  void onMessage(const ConnectionPtr& conn);

  std::shared_ptr<Message> set(const std::string& key, int32_t exptime, const std::string& value);

  std::shared_ptr<Message> get(const std::string& key);

private:
  Client client_;
  CountDownLatch& countDownLatch_;
  ConnectionPtr connectionPtr_;

};

#endif