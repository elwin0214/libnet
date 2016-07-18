#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include "command.h"
#include "message.h"
#include "memcached_client.h"

using namespace libnet;

class Context : public NoCopyable
{
public:
  typedef std::queue<std::shared_ptr<Message>> Queue;
  Context():lock_()
  {

  }

  std::shared_ptr<Message>& front()
  {
    LockGuard guard(lock_);
    return queue_.front();
  }

  void pop()
  {
    LockGuard guard(lock_);
    queue_.pop();
  }

  void push(std::shared_ptr<Message>& message)
  {
    LockGuard guard(lock_);
    queue_.push(message);
  }

private:
  MutexLock lock_;
  Queue queue_;

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
    countDownLatch_.countDown();
  }
  else if (conn->disconnected())
  {
    Context* ctx = static_cast<Context*>(conn->getContext());
    delete ctx;
  }
};

std::shared_ptr<Message> MemcachedClient::set(const std::string& key, int32_t exptime, const std::string& value)
{
  std::shared_ptr<Message> message(new Message(new SetCommand(key, exptime, value)));
  Context* context = static_cast<Context*>(connectionPtr_->getContext());

  context->push(message);
  Buffer* buffer = new Buffer(0, 1024);
  message->append(*buffer);
  LOG_TRACE << " send = " << (buffer->toString()) ;
  connectionPtr_->sendBuffer(buffer);
  return message;
};

std::shared_ptr<Message> MemcachedClient::get(const std::string& key)
{
  std::shared_ptr<Message> message(new Message(new GetCommand(key)));
  Context* context = static_cast<Context*>(connectionPtr_->getContext());

  context->push(message);
  Buffer* buffer = new Buffer(0, 1024);
  message->append(*buffer);
  LOG_TRACE << " send = " << (buffer->toString()) ;
  connectionPtr_->sendBuffer(buffer);
  return message;
};


void MemcachedClient::onMessage(const ConnectionPtr& conn)
{
  Buffer& input = conn->input();
  LOG_DEBUG << "read=" << input.toString() ;
  Context* context = static_cast<Context*>(connectionPtr_->getContext());
  std::shared_ptr<Message>& message = context->front();
  message->parse(input);
  if (message->code() != kNeedMore)
  {
    context->pop();
    message->wakeup();
  }
};