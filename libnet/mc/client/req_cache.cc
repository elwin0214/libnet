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
#include <libnet/mc/req_cache.h>

namespace mc
{
namespace client
{
using namespace libnet;
using namespace std;

RequestCache::RequestCache(size_t buffer_size)
              : sending_lock_(),
                sent_lock_(),
                sending_cond_(sending_lock_),
                reject_sending_(false),
                closed_(false),
                sending_queue_(),
                sent_queue_(),
                buffer_size_(buffer_size)
{
};

void RequestCache::start()
{
  closed_ = false; 
  reject_sending_ = false;
};

void RequestCache::close()
{
  
  LOG_TRACE << "close()";
  {
    LockGuard guard(sending_lock_);
    closed_ = true;
    sending_cond_.notifyAll();
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
};

// collect more command and write to connection at a time
size_t RequestCache::write(const Conn& conn)
{   
  vector<shared_ptr<Command>> commands;
  Buffer buffer(0, buffer_size_);
  size_t n = 0;
  size_t remain = 0;
  {
    LockGuard guard(sending_lock_); 
    if (sending_queue_.empty()) return 0;
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
    remain = sending_queue_.size();
    // if (!sending_queue_.empty())
    // {
    //   conn->loop()->queueInLoop(std::bind(&RequestCache::writeRequest, this ,conn));//if dont write all command
    // }
  }

  if (commands.size() <= 0) return remain;
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
    conn->sendBuffer(&buffer);
  return remain;
};

bool RequestCache::send(const std::shared_ptr<Command>& cmd, int32_t send_wait_milli, bool check_cache_reject)
{
  LOG_DEBUG << send_wait_milli << " " << check_cache_reject << " stoped = " << closed_.load();
  if (closed_) return false;
  LockGuard guard(sending_lock_);
  if (closed_) return false;//
  if (!check_cache_reject)
  {
    sending_queue_.push(cmd);
    return true;
  }
  while (reject_sending_)
  {
    if (send_wait_milli <= 0) return false;
    send_wait_milli = sending_cond_.wait(send_wait_milli);
    if (closed_) return false;
  }
  sending_queue_.push(cmd);
  return true;
};

bool RequestCache::receive(Buffer& input)
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
};

void RequestCache::acceptSending()
{
  reject_sending_ = false;
  sending_cond_.notifyAll();
};

void RequestCache::rejectSending()
{
  reject_sending_ = true;
};

}
}