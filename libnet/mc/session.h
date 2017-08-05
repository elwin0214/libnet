#ifndef __LIBNET_MC_CLIENT_SESSION_H__
#define __LIBNET_MC_CLIENT_SESSION_H__
#include <memory>
#include <atomic>
#include <future>
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/client.h>
#include <libnet/countdown_latch.h>
#include <libnet/mc/message.h>
#include <libnet/mc/command.h>

namespace mc
{
namespace client
{
using namespace libnet;
using namespace std;
using namespace mc::msg;
class RequestCache;
// a tcp connection 

class Session : public NoCopyable 
{
public:
  typedef std::shared_ptr<Connection> Conn;
  typedef std::shared_ptr<Message> Msg;

public:
  Session(EventLoop* loop, 
          const char* host, 
          uint16_t port, 
          CountDownLatch& connected_latch_, 
          CountDownLatch& closed_latch_, 
          uint32_t idle_timeout = 1000,
          size_t max_retry = 2);

  Session(EventLoop* loop, 
          const InetAddress& remote_addr, 
          CountDownLatch& connected_latch_, 
          CountDownLatch& closed_latch_,
          uint32_t idle_timeout = 1000,
          size_t max_retry = 2);

  void connect();
  void disconnect();
  bool connected() { return connected_; }
  void enableRetry() { client_.enableRetry(); }

  void onConnection(const Conn& conn);
  void onMessage(const Conn& conn);

  void send(const std::shared_ptr<Command>& cmd);

  void writeRequest();
  void notifyWrite();

private:
  void onIdle();
  void update();

private:
  EventLoop* loop_;
  Client client_;  
  shared_ptr<RequestCache> cache_;
  CountDownLatch& connected_latch_;
  CountDownLatch& closed_latch_;
  TimerId timer_id_;
  Conn conn_;
  std::atomic<bool> connected_;
  uint64_t last_optime_;  // ms
  uint32_t idle_timeout_; // ms
  size_t max_retry_; 
  size_t retries_;

};

}
}

#endif