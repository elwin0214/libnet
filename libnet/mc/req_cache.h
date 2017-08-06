#ifndef __LIBNET_MC_CLIENT_CACHE_H__
#define __LIBNET_MC_CLIENT_CACHE_H__
#include <queue>
#include <atomic>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>

namespace mc
{
namespace msg
{
class Command;
}
namespace client
{
using namespace libnet;
using namespace std;
using namespace mc::msg;

// store the sending command and sent command
class RequestCache : public NoCopyable
{
public:
  typedef std::queue<std::shared_ptr<Command>> CommandQueue;
  typedef shared_ptr<Connection> Conn;

  RequestCache(size_t buffer_size = 4096);
  
  void start() { stoped_ = false; }
  void close();

  void writeRequest(const Conn& connection);

  bool push(const std::shared_ptr<Command>& cmd, int32_t send_wait_milli, bool check_cache_reject);

  bool readResponse(Buffer& input);

  void disableSending(const Conn& conn, size_t size)
  {
    LOG_DEBUG << "conn = " << conn->id() << " size = " << size ;
    reject_sending_ = true;
  }

  void enableSending(const Conn& conn)
  {
    LOG_DEBUG << "conn = " << conn->id();
    LockGuard guard(sending_lock_);
    reject_sending_ = false;
    sending_cond_.notifyAll();
  }
private:
  MutexLock sending_lock_;
  MutexLock sent_lock_;
  Condition sending_cond_;
  std::atomic<bool> reject_sending_;
  std::atomic<bool> stoped_; // used to release the thread wait on sending_queue_
  CommandQueue sending_queue_;
  CommandQueue sent_queue_;
  size_t buffer_size_;


};

}
}

#endif