#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>
#include "command.h"
#include "message.h"
#include "memcached_client.h"

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

void MemcachedClient::onConnection(const ConnectionPtr& conn)
{
  if (conn->connected())
  {
    Context* ctx = new Context();
    conn->setContext(ctx);
    connectionPtr_ = conn;
    latch_.countDown();
  }
  else if (conn->disconnected())
  {
    Context* ctx = static_cast<Context*>(conn->getContext());
    delete ctx;
  }
};

void MemcachedClient::send(const std::shared_ptr<Message>& message)
{
  Context* context = static_cast<Context*>(connectionPtr_->getContext());
  context->push(message);
  notify();
};

void MemcachedClient::onMessage(const ConnectionPtr& conn)
{
  Buffer& input = conn->input();
  LOG_TRACE << "read=" << input.toString() ;
  Context* context = static_cast<Context*>(connectionPtr_->getContext());

  while(context->parse(input))// more response
  {
  }

};

void MemcachedClient::write()
{
  (connectionPtr_->loop()->assertInLoopThread());
  Context* context = static_cast<Context*>(connectionPtr_->getContext());
  context->write(connectionPtr_);
};

void MemcachedClient::notify()
{
  connectionPtr_->loop()->runInLoop(std::bind(&MemcachedClient::write, this));
}


