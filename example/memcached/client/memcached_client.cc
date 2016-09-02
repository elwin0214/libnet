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
  typedef std::queue<std::shared_ptr<Message>> CommandQueue;

  Context():lock_()
  {
    
  }

  ~Context()
  {
    LockGuard guard(lock_);
    while (!commandSendingQueue_.empty())
    {
      commandSendingQueue_.front()->wakeup();
      commandSendingQueue_.pop();
    }
    while (!commandSentQueue_.empty())
    {
      commandSentQueue_.front()->wakeup();
      commandSentQueue_.pop();
    }
  }

  void write(const MemcachedClient::ConnectionPtr& connectionPtr)
  {
    LockGuard guard(lock_);
    while (!commandSendingQueue_.empty())
    {
      Buffer* buffer = new Buffer(0, 1024);
      std::shared_ptr<Message>& messagePtr = commandSendingQueue_.front();
      messagePtr->append(*buffer);
      connectionPtr->sendBuffer(buffer);
      commandSentQueue_.push(messagePtr); 
      commandSendingQueue_.pop();
    }
  }

  void push(const std::shared_ptr<Message>& message)
  {
    LockGuard guard(lock_);
    commandSendingQueue_.push(message);
  }

  bool parse(Buffer& input)
  {
    LockGuard guard(lock_);
    if (commandSentQueue_.empty()){
      assert(input.readable() <= 0);// 多余数据
      return false;
    }
    std::shared_ptr<Message>& messagePtr = commandSentQueue_.front();
    if (messagePtr->parse(input))
    {
      LOG_DEBUG << "parse.wakup" ;
      messagePtr->wakeup();
      commandSentQueue_.pop();
      //todo
      return true;
    }

    return false;
  }

private:
  MutexLock lock_;
  CommandQueue commandSendingQueue_;
  CommandQueue commandSentQueue_;
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

void MemcachedClient::send(const std::shared_ptr<Message>& message)
{
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
  context->push(message);
  notify();
};

void MemcachedClient::onMessage(const ConnectionPtr& connection)
{
  Buffer& input = connection->input();
  LOG_TRACE << "read=" << input.toString() ;

  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection->getContext());

  while(context->parse(input))// more response
  {
  }

};

void MemcachedClient::write()
{
  (connection_->loop()->assertInLoopThread());
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
  context->write(connection_);
};

void MemcachedClient::notify()
{
  connection_->loop()->runInLoop(std::bind(&MemcachedClient::write, this));
};

}
}
