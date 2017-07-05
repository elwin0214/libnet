#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>
#include "command.h"
#include "async_client.h"

namespace memcached
{
namespace client
{

using namespace libnet;
class Context : public NoCopyable
{
public:
  typedef std::queue<std::shared_ptr<Command>> CommandQueue;

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

  void write(const AsyncClient::ConnectionPtr& connection)
  {    
    Buffer buffer(0, 4096);
    for (int i = 0; i < 10; i++)
    {
      std::shared_ptr<Command> cmd;
      {
        LockGuard guard(sending_lock_);
        if (sending_queue_.empty())
          break;
        cmd = sending_queue_.front();
        sending_queue_.pop();
      }
      cmd->encode(buffer);
      {
        LockGuard guard(sent_lock_);
        sent_queue_.push(cmd);
      }
    }
    if (log::Logger::getLogLevel() <= libnet::log::TRACE)
    {
      LOG_TRACE << "write = " << buffer.toString() ;
    }
    if (buffer.readable() > 0)
      connection->sendBuffer(&buffer);
  }

  void push(const std::shared_ptr<Command>& cmd)
  {
    LockGuard guard(sending_lock_);
    sending_queue_.push(cmd);
  }

  bool parse(Buffer& input)
  {
    //LockGuard guard(lock_);
    LockGuard guard(sent_lock_);
    if (sent_queue_.empty()){
      assert(input.readable() <= 0);// 收到了server发来的多余数据
      return false;
    }
    std::shared_ptr<Command>& cmd = sent_queue_.front();
    if (cmd->decode(input))
    {
      LOG_DEBUG << "parse.wakup" ;
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
};

void AsyncClient::connect()
{
  client_.setConnectionCallBack(std::bind(&AsyncClient::onConnection, this, std::placeholders::_1));
  client_.setMessageCallBack(std::bind(&AsyncClient::onMessage, this, std::placeholders::_1));
  client_.connect();
};

void AsyncClient::onConnection(const ConnectionPtr& connection)
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

void AsyncClient::send(const std::shared_ptr<Command>& message)
{
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
  context->push(message);
  notify();
};

void AsyncClient::onMessage(const ConnectionPtr& connection)
{ 
  connection_->loop()->assertInLoopThread();
  Buffer& input = connection->input();
  LOG_TRACE << "read=" << input.toString() ;
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection->getContext());
  while(context->parse(input))// more response
  {
  }

};

void AsyncClient::write()
{
  connection_->loop()->assertInLoopThread();
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
  context->write(connection_);
};

void AsyncClient::notify()
{
  connection_->loop()->runInLoop(std::bind(&AsyncClient::write, this));
};

}
}
