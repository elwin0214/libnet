#ifndef __LIBNET_MC_CLIENT_CACHE_H__
#define __LIBNET_MC_CLIENT_CACHE_H__
#include <queue>
#include <atomic>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>

namespace mc
{
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

  RequestCache(size_t capacity = 20000, size_t batch_buffer_size = 10240);
  
  void start();
  void close();

  bool send(const std::shared_ptr<Command>& cmd, int32_t send_wait_milli, bool check_cache_reject);
  size_t write(const Conn& connection);
  bool receive(Buffer& input);

  // void acceptSending();
  // void rejectSending();

private:
  MutexLock sending_lock_;
  MutexLock sent_lock_;
  Condition sending_cond_;
  
  std::atomic<size_t> size_;
  //std::atomic<bool> reject_sending_;
  std::atomic<bool> closed_; // used to release the thread wait on sending_queue_
  CommandQueue sending_queue_;
  size_t capacity_;
  CommandQueue sent_queue_;
  size_t batch_buffer_size_;

};

}
}

#endif