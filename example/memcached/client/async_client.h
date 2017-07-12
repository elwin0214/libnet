#ifndef __LIBNET_MEMCACHED_ASYNCCLIENT_H__
#define __LIBNET_MEMCACHED_ASYNCCLIENT_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include <libnet/mutexlock.h>
#include <string>
#include <future>
#include "command.h"

namespace memcached
{
namespace client
{

using namespace libnet;
using namespace std;

class ClientImpl;
class AsyncClient : public NoCopyable
{
public:
  typedef std::shared_ptr<ClientImpl> Impl;
  typedef std::shared_ptr<Message> Msg;

  AsyncClient(EventLoop* loop, const char* host, int port, CountDownLatch& latch, int connSize = 1);

  void connect();

  void disconnect();

  std::future<Msg> get(const string& key)
  {
    return sendFuture(Message(kGet, key));
  };

  std::future<Msg> set(const std::string& key, const std::string& value, int32_t exptime)
  {
    return sendFuture(Message(kSet, key, value, 0, exptime)); 
  };

  std::future<Msg> add(const std::string& key, const std::string& value, int32_t exptime)
  {
    return sendFuture(Message(kAdd, key, value, 0, exptime)); 
  };

  std::future<Msg> replace(const std::string& key, const std::string& value, int32_t exptime)
  {
    return sendFuture(Message(kReplace, key, value, 0, exptime)); 
  };

  std::future<Msg> remove(const std::string& key)
  {
    return sendFuture(Message(kDelete, key)); 
  };

  std::future<Msg> incr(const std::string& key, uint32_t value)
  {
    return sendFuture(Message(kIncr, key, value)); 
  };

  std::future<Msg> decr(const std::string& key, uint32_t value)
  {
    return sendFuture(Message(kDecr, key, value)); 
  };

  future<Msg> sendFuture(Message request);

  void send(const std::shared_ptr<Command>& command);

  int random(const int& min, const int& max);

private:
  std::vector<Impl> impls;
  CountDownLatch& latch_;
  MutexLock randLock_;
};

}
}
#endif