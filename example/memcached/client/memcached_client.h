#ifndef __LIBNET_MEMCACHED_CLIENT_H__
#define __LIBNET_MEMCACHED_CLIENT_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include <libnet/mutexlock.h>
#include "message.h"

namespace memcached
{
namespace client
{

using namespace libnet;
using namespace std;

class MemcachedClient : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;
  MemcachedClient(EventLoop* loop, const char* host, int port, CountDownLatch& latch)
    : client_(loop, host, port),
      latch_(latch)
  {
  };

  void connect();

  void disconnect(){ client_.disconnect(); }

  shared_ptr<Future<std::string>> get(const string& key)
  {
    shared_ptr<Future<std::string>> future(new Future<std::string>());
    shared_ptr<Message<std::string>> message(new Message<std::string>(new GetCommand(key), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future<bool>> set(const std::string& key, int32_t exptime, const std::string& value)
  {
    shared_ptr<Future<bool>> future(new Future<bool>());
    std::shared_ptr<Message<bool>> message(new Message<bool>(new TextStoreCommand("set", key, exptime, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future<bool>> add(const std::string& key, int32_t exptime, const std::string& value)
  {
    shared_ptr<Future<bool>> future(new Future<bool>());
    std::shared_ptr<Message<bool>> message(new Message<bool>(new TextStoreCommand("add", key, exptime, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future<bool>> replace(const std::string& key, int32_t exptime, const std::string& value)
  {
    shared_ptr<Future<bool>> future(new Future<bool>());
    std::shared_ptr<Message<bool>> message(new Message<bool>(new TextStoreCommand("replace", key, exptime, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future<bool>> remove(const std::string& key)
  {
    shared_ptr<Future<bool>> future(new Future<bool>());
    std::shared_ptr<Message<bool>> message(new Message<bool>(new DeleteCommand(key), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future<uint32_t>> incr(const std::string& key, uint32_t value)
  {
    shared_ptr<Future<uint32_t>> future(new Future<uint32_t>());
    std::shared_ptr<Message<uint32_t>> message(new Message<uint32_t>(new CountCommand("incr", key, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future<uint32_t>> decr(const std::string& key, uint32_t value)
  {
    shared_ptr<Future<uint32_t>> future(new Future<uint32_t>());
    std::shared_ptr<Message<uint32_t>> message(new Message<uint32_t>(new CountCommand("decr", key, value), future));
    send(message);
    return future;
  };

private:
  void onConnection(const ConnectionPtr& conn);
  void onMessage(const ConnectionPtr& conn);
  void send(const std::shared_ptr<Caller>& message);
  void write();
  void notify();

private:
  Client client_;
  CountDownLatch& latch_;
  ConnectionPtr connection_;

};

}
}
#endif