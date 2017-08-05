#ifndef __LIBNET_MC_CLIENT_CACHE_H__
#define __LIBNET_MC_CLIENT_CACHE_H__
#include <queue>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>

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
// store the sending command and sent command
class RequestCache : public NoCopyable
{
public:
  typedef std::queue<std::shared_ptr<Command>> CommandQueue;
  typedef shared_ptr<Connection> Conn;

  RequestCache(int buffer_size = 4096)
    : sending_lock_(),
      sent_lock_(),
      sending_queue_(),
      sent_queue_(),
      buffer_size_(buffer_size)
  {
    assert(buffer_size > 0);
  }
  
  void clear();

  void writeRequest(const Conn& connection);

  void push(const std::shared_ptr<Command>& cmd);

  bool readResponse(Buffer& input);

private:
  MutexLock sending_lock_;
  MutexLock sent_lock_;
  CommandQueue sending_queue_;
  CommandQueue sent_queue_;
  int buffer_size_;
};

}
}

#endif