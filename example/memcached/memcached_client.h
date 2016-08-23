#ifndef __LIBNET_MEMCACHED_CLIENT_H__
#define __LIBNET_MEMCACHED_CLIENT_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include <libnet/mutexlock.h>
#include "message.h"

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

  shared_ptr<Future> get(const string& key)
  {
    shared_ptr<Future> future(new Future());
    shared_ptr<Message> message(new Message(new GetCommand(key), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future> set(const std::string& key, int32_t exptime, const std::string& value)
  {
    shared_ptr<Future> future(new Future());
    std::shared_ptr<Message> message(new Message(new TextStoreCommand("set", key, exptime, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future> add(const std::string& key, int32_t exptime, const std::string& value)
  {
    shared_ptr<Future> future(new Future());
    std::shared_ptr<Message> message(new Message(new TextStoreCommand("add", key, exptime, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future> replace(const std::string& key, int32_t exptime, const std::string& value)
  {
    shared_ptr<Future> future(new Future());
    std::shared_ptr<Message> message(new Message(new TextStoreCommand("replace", key, exptime, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future> remove(const std::string& key)
  {
    shared_ptr<Future> future(new Future());
    std::shared_ptr<Message> message(new Message(new DeleteCommand(key), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future> incr(const std::string& key, uint32_t value)
  {
    shared_ptr<Future> future(new Future());
    std::shared_ptr<Message> message(new Message(new CountCommand("incr", key, value), future));
    send(message);
    return future;
  };

  std::shared_ptr<Future> decr(const std::string& key, uint32_t value)
  {
    shared_ptr<Future> future(new Future());
    std::shared_ptr<Message> message(new Message(new CountCommand("decr", key, value), future));
    send(message);
    return future;
  };

private:
  void onConnection(const ConnectionPtr& conn);
  void onMessage(const ConnectionPtr& conn);
  void send(const std::shared_ptr<Message>& message);
  void write();
  void notify();

private:
  Client client_;
  CountDownLatch& latch_;
  ConnectionPtr connectionPtr_;

};

#endif