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

  bool get(const string& key, string& result)
  {
    Message request(kGet, key);
    CountDownLatch latch(1);
    shared_ptr<Command> cmd = make_shared<Command>(request, [&latch](){ latch.countDown(); }); 
    send(cmd);
    latch.wait();
    Message& response = cmd->response();
    result = response.data_.value_;
    return response.stat_.code_ == kSucc;
  };

  bool set(const std::string& key, int32_t exptime, const std::string& value)
  {
    CountDownLatch latch(1);
    Message request(kSet, key, value, 0, exptime);
    shared_ptr<Command> cmd = make_shared<Command>(request, [&latch](){ latch.countDown(); }); 
    send(cmd);
    latch.wait();
    Message& response = cmd->response();
    return response.stat_.code_ == kSucc;
  };

  bool add(const std::string& key, int32_t exptime, const std::string& value)
  {
    CountDownLatch latch(1);
    Message request(kAdd, key, value, 0, exptime);
    shared_ptr<Command> cmd = make_shared<Command>(request, [&latch](){ latch.countDown(); }); 
    send(cmd);
    latch.wait();
    Message& response = cmd->response();
    return response.stat_.code_ == kSucc;
  };

  bool replace(const std::string& key, int32_t exptime, const std::string& value)
  {
    CountDownLatch latch(1);
    Message request(kReplace, key, value, 0, exptime);
    shared_ptr<Command> cmd = make_shared<Command>(request, [&latch](){ latch.countDown(); }); 
    send(cmd);
    latch.wait();
    Message& response = cmd->response();
    return response.stat_.code_ == kSucc;
  };

  bool remove(const std::string& key)
  {
    CountDownLatch latch(1);
    Message request(kDelete, key);
    shared_ptr<Command> cmd = make_shared<Command>(request, [&latch](){ latch.countDown(); }); 
    send(cmd);
    latch.wait();
    Message& response = cmd->response();
    return response.stat_.code_ == kSucc;
  };

  bool incr(const std::string& key, uint32_t value, uint32_t& result)
  {
    CountDownLatch latch(1);
    Message request(kIncr, key, value);
    shared_ptr<Command> cmd = make_shared<Command>(request, [&latch](){ latch.countDown(); }); 
    send(cmd);
    latch.wait();
    Message& response = cmd->response();
    result = response.data_.count_;
    return response.stat_.code_ == kSucc;
  };

  bool decr(const std::string& key, uint32_t value, uint32_t& result)
  {
    CountDownLatch latch(1);
    Message request(kIncr, key, value);
    shared_ptr<Command> cmd = make_shared<Command>(request, [&latch](){ latch.countDown(); }); 
    send(cmd);
    latch.wait();
    Message& response = cmd->response();
    result = response.data_.count_;
    return response.stat_.code_ == kSucc;
  };

  void send(const std::shared_ptr<Command>& command);

private:
  void onConnection(const ConnectionPtr& conn);
  void onMessage(const ConnectionPtr& conn);
  void writeRequest();
  void notifyWrite();

private:
  Client client_;
  CountDownLatch& latch_;
  ConnectionPtr connection_;

};

}
}
#endif