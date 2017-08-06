#ifndef __LIBNET_MC_ASYNCCLIENT_H__
#define __LIBNET_MC_ASYNCCLIENT_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include <libnet/mutexlock.h>
#include <string>
#include <future>
#include "command.h"

namespace mc
{
namespace client
{

using namespace libnet;
using namespace std;

class Session;
class AsyncClient : public NoCopyable
{
public:
  typedef std::shared_ptr<Message> Msg;

  AsyncClient(EventLoop* loop, 
              const char* host, 
              uint16_t port, 
              CountDownLatch& connected_latch_,
              CountDownLatch& closed_latch_,
              size_t conn_size = 1,
              int32_t send_wait_milli = 5000,
              size_t high_water_mark = 4096,
              uint32_t idle_timeout_milli = 20000,
              size_t max_retry = 3);

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

  void enableRetry();
  
private:
  std::vector<shared_ptr<Session>> sessions;
  int32_t send_wait_milli_;
  //size_t high_water_mark_;
  //uint32_t idle_timeout_milli_;
  //size_t max_retry_;
};

}
}
#endif