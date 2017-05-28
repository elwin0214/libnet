#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>
#include "command.h"
#include "message.h"
#include "memcached_client.h"

namespace memcached
{
namespace client
{

using namespace libnet;
class Context : public NoCopyable
{
public:
  typedef std::queue<std::shared_ptr<Caller>> CommandQueue;

  Context():sending_lock_(),sent_lock_()
  {
    
  }

  ~Context()
  {
    {
      LockGuard guard(sending_lock_);
      while (!sending_queue_.empty())
      {
        sending_queue_.front()->wakeup();
        sending_queue_.pop();
      }
    }

    {
      LockGuard guard(sent_lock_);
      while (!sent_queue_.empty())
      {
        sent_queue_.front()->wakeup();
        sent_queue_.pop();
      }
    }
  }

  void write(const MemcachedClient::ConnectionPtr& connection)
  {    
    Buffer buffer(0, 2048);
    for (int i = 0; i < 10; i++)
    {
      std::shared_ptr<Caller> caller;
      {
        LockGuard guard(sending_lock_);
        if (sending_queue_.empty())
          break;
        caller = sending_queue_.front();
        sending_queue_.pop();
      }
      caller->append(buffer);
      {
        LockGuard guard(sent_lock_);
        sent_queue_.push(caller);
      }
    }
    if (log::Logger::getLogLevel() <= libnet::log::TRACE)
    {
      LOG_TRACE << "write = " << buffer.toString() ;
    }
    if (buffer.readable() > 0)
      connection->sendBuffer(&buffer);
  }

  void push(const std::shared_ptr<Caller>& message)
  {
    LockGuard guard(sending_lock_);
    sending_queue_.push(message);
  }

  bool parse(Buffer& input)
  {
    //LockGuard guard(lock_);
    LockGuard guard(sent_lock_);
    if (sent_queue_.empty()){
      assert(input.readable() <= 0);// 收到了server发来的多余数据
      return false;
    }
    std::shared_ptr<Caller>& caller = sent_queue_.front();
    if (caller->parse(input))
    {
      LOG_DEBUG << "parse.wakup" ;
      caller->wakeup();
      sent_queue_.pop();
      //todo
      return true;
    }

    return false;
  }

private:
  MutexLock sending_lock_;
  MutexLock sent_lock_;
  CommandQueue sending_queue_;
  CommandQueue sent_queue_;
};

void MemcachedClient::connect()
{
  client_.setConnectionCallBack(std::bind(&MemcachedClient::onConnection, this, std::placeholders::_1));
  client_.setMessageCallBack(std::bind(&MemcachedClient::onMessage, this, std::placeholders::_1));
  client_.connect();
};

void MemcachedClient::onConnection(const ConnectionPtr& connection)
{
  if (connection->connected())
  {
    std::shared_ptr<Context> context = std::make_shared<Context>();
    connection->setContext(context);
    connection_ = connection;
    latch_.countDown();
  }
  else if (connection->disconnected())
  {
  }
};

void MemcachedClient::send(const std::shared_ptr<Caller>& message)
{
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
  context->push(message);
  notify();
};

void MemcachedClient::onMessage(const ConnectionPtr& connection)
{ 
  connection_->loop()->assertInLoopThread();
  Buffer& input = connection->input();
  LOG_TRACE << "read=" << input.toString() ;
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection->getContext());
  while(context->parse(input))// more response
  {
  }

};

void MemcachedClient::write()
{
  connection_->loop()->assertInLoopThread();
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
  context->write(connection_);
};

void MemcachedClient::notify()
{
  connection_->loop()->runInLoop(std::bind(&MemcachedClient::write, this));
};

}
}
