#ifndef __LIBNET_MEMCACHED_ASYNCCLIENT_H__
#define __LIBNET_MEMCACHED_ASYNCCLIENT_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include <libnet/mutexlock.h>
#include <string>
#include "command.h"

namespace memcached
{
namespace client
{

using namespace libnet;
using namespace std;

class AsyncClient : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

  AsyncClient(EventLoop* loop, const char* host, int port, CountDownLatch& latch)
    : client_(loop, host, port),
      latch_(latch)
  {
  };

  void connect();

  void disconnect(){ client_.disconnect(); }

  string get(const string& key)
  {
    CountDownLatch latch(1);
    Request request(kGet, key);
    shared_ptr<Command> msg(new Command(std::move(request), latch)); 
    send(msg);
    latch.wait();
    return (msg->response())->value_;
  };

  bool set(const std::string& key, int32_t exptime, const std::string& value)
  {
    CountDownLatch latch(1);
    Request request(kSet, key, value, 0, exptime);
    shared_ptr<Command> msg(new Command(std::move(request), latch)); 
    send(msg);
    latch.wait();
    return (msg->response())->code_ == kSucc;
  };

  bool add(const std::string& key, int32_t exptime, const std::string& value)
  {
    CountDownLatch latch(1);
    Request request(kAdd, key, value, 0, exptime);
    shared_ptr<Command> msg(new Command(std::move(request), latch)); 
    send(msg);
    latch.wait();
    return (msg->response())->code_ == kSucc;
  };

  bool replace(const std::string& key, int32_t exptime, const std::string& value)
  {
    CountDownLatch latch(1);
    Request request(kReplace, key, value, 0, exptime);
    shared_ptr<Command> msg(new Command(std::move(request), latch)); 
    send(msg);
    latch.wait();
    return (msg->response())->code_ == kSucc;
  };

  bool remove(const std::string& key)
  {
    CountDownLatch latch(1);
    Request request(kDelete, key);
    shared_ptr<Command> msg(new Command(std::move(request), latch)); 
    send(msg);
    latch.wait();
    return (msg->response())->code_ == kSucc;
  };

  uint32_t incr(const std::string& key, uint32_t value)
  {
    CountDownLatch latch(1);
    Request request(kIncr, key, value);
    shared_ptr<Command> msg(new Command(std::move(request), latch)); 
    send(msg);
    latch.wait();
    return (msg->response())->count_;
  };

  uint32_t decr(const std::string& key, uint32_t value)
  {
    CountDownLatch latch(1);
    Request request(kDecr, key, value);
    shared_ptr<Command> msg(new Command(std::move(request), latch)); 
    send(msg);
    latch.wait();
    return (msg->response())->count_;
  };

private:
  void onConnection(const ConnectionPtr& conn);
  void onMessage(const ConnectionPtr& conn);
  void send(const std::shared_ptr<Command>& message);
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