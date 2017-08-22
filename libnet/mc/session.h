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
#include <libnet/mc/req_cache.h>

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
          CountDownLatch& connected_latch, 
          CountDownLatch& closed_latch,
          size_t high_water_mark = 4096,
          uint32_t idle_timeout_milli = 20000,
          size_t max_retry = 3);

  Session(EventLoop* loop, 
          const InetAddress& remote_addr, 
          CountDownLatch& connected_latch, 
          CountDownLatch& closed_latch,
          size_t high_water_mark = 4096,
          uint32_t idle_timeout_milli = 20000,
          size_t max_retry = 3);

  void connect();
  void disconnect();
  bool connected() { return connected_; }
  void enableRetry() { client_.enableRetry(); }

  void onConnection(const Conn& conn);
  void onMessage(const Conn& conn);

  // if check_cache_reject is false , the idle_timeout_milli is not valid
  bool send(const std::shared_ptr<Command>& cmd, int32_t send_wait_milli, bool check_cache_reject = true);

  void writeInLoop();
  void notifyWrite();

private:
  void acceptWrite(const Conn& conn);
  void rejectWrite(const Conn& conn, size_t size);

  void onIdle();
  void update();

private:
  EventLoop* loop_;
  Client client_;  
  RequestCache cache_;
  CountDownLatch& connected_latch_;
  CountDownLatch& closed_latch_;
  size_t high_water_mark_;
  TimerId timer_id_;
  Conn conn_;
  std::atomic<bool> connected_;

  uint64_t last_optime_;  // ms
  uint32_t idle_timeout_milli_; // ms
  size_t max_retry_; 
  size_t retries_;
};

}
}

#endif