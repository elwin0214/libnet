#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>
#include <random>
#include <memory>
#include <iostream>
#include <libnet/mc/command.h>

namespace mc
{
namespace client
{
using namespace libnet;
using namespace std;

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
  
  void clear()
  {
    LOG_TRACE << "clear()";
    {
      LockGuard guard(sending_lock_);
      while (!sending_queue_.empty())
      {
        auto cmd = sending_queue_.front();
        cmd->response().stat_.code_ = kConClosed;
        sending_queue_.front()->call();
        sending_queue_.pop();
      }
    }
    {
      LockGuard guard(sent_lock_);
      while (!sent_queue_.empty())
      {
        auto cmd = sent_queue_.front();
        cmd->response().stat_.code_ = kConClosed;
        sent_queue_.front()->call();
        sent_queue_.pop();
      }
    }
  }

  void writeRequest(const Conn& connection)
  {   
    vector<shared_ptr<Command>> commands;
    Buffer buffer(0, buffer_size_);
    int n = 0;
    {
      LockGuard guard(sending_lock_); 
      if (sending_queue_.empty()) return;
      commands.reserve(1024);
      for (n = 0; n < 1024; n++)
      {
        if (sending_queue_.empty() || buffer.readable() >= buffer_size_)
          break;
        std::shared_ptr<Command> cmd = sending_queue_.front();
        sending_queue_.pop();
        cmd->encode(buffer);
        commands.push_back(std::move(cmd));
      }
    }

    if (commands.size() <= 0) return;
    {
      LockGuard guard(sent_lock_);
      for (auto itr = commands.begin(); itr != commands.end(); itr++)
      {
        sent_queue_.push(std::move(*itr));
      }
    }
    if (log::Logger::getLogLevel() <= libnet::log::TRACE)
    {
      LOG_TRACE << "n = " << n << " buffer.readable = " << buffer.readable();
    }
    if (buffer.readable() > 0)
      connection->sendBuffer(&buffer);
  }

  void push(const std::shared_ptr<Command>& cmd)
  {
    LockGuard guard(sending_lock_);
    sending_queue_.push(cmd);
  }

  bool readResponse(Buffer& input)
  {
    LockGuard guard(sent_lock_);
    if (sent_queue_.empty()){
      assert(input.readable() <= 0);// 收到了server发来的多余数据
      return false;
    }
    std::shared_ptr<Command>& cmd = sent_queue_.front();
    if (cmd->decode(input))
    {
      LOG_DEBUG << "read.wakup" ;
      sent_queue_.pop();
      return true;
    }
    return false;
  }

private:
  MutexLock sending_lock_;
  MutexLock sent_lock_;
  CommandQueue sending_queue_;
  CommandQueue sent_queue_;
  int buffer_size_;
};

}
}