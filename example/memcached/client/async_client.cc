#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/countdown_latch.h>
#include <libnet/nocopyable.h>
#include <libnet/mutexlock.h>
#include <libnet/logger.h>
#include <random>
#include <memory>
#include "command.h"
#include "async_client.h"

namespace memcached
{
namespace client
{
using namespace libnet;
using namespace std;

// store the sending command and sent command
class Cache : public NoCopyable
{
public:
  typedef std::queue<std::shared_ptr<Command>> CommandQueue;
  typedef shared_ptr<Connection> ConnectionPtr;
  Cache()
    : sending_lock_(),
      sent_lock_(),
      sending_queue_(),
      sent_queue_()
  {
  }
  ~Cache()
  {
    LOG_TRACE << "~Cache()";
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
        auto cmd = sending_queue_.front();
        cmd->response().stat_.code_ = kConClosed;
        sent_queue_.front()->call();
        sent_queue_.pop();
      }
    }
  }

  void writeRequest(const ConnectionPtr& connection)
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
};
// a tcp connection 
class ClientImpl : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

private:
  Client client_;
  CountDownLatch& latch_;
  ConnectionPtr connection_;
  Cache cache_;

public:
  ClientImpl(EventLoop* loop, const char* host, int port, CountDownLatch& latch)
    : client_(loop, host, port),
      latch_(latch),
      connection_(),
      cache_()
  {

  }

  void connect()
  {
    client_.setConnectionCallBack(std::bind(&ClientImpl::onConnection, this, std::placeholders::_1));
    client_.setMessageCallBack(std::bind(&ClientImpl::onMessage, this, std::placeholders::_1));
    client_.connect();
  }

  void disconnect()
  { 
    connection_.reset(); 
    client_.disconnect(); 
  }

  void onConnection(const ConnectionPtr& connection)
  {
    if (connection->connected())
    {
      //std::shared_ptr<Context> context = std::make_shared<Context>();
      //connection->setContext(context);
      connection_ = connection;
      latch_.countDown();
    }
    else if (connection->disconnected())
    {
      //connection_.reset();
    }
  }

  future<AsyncClient::Msg> sendFuture(Message request)
  {
    auto p = make_shared<std::promise<AsyncClient::Msg>>() ;
    auto f = p->get_future(); 
    shared_ptr<Command> cmd = make_shared<Command>(request, [p](const AsyncClient::Msg& msg)mutable{ p->set_value(msg); });
    send(cmd);
    return f;//NRVO
  }

  void send(const std::shared_ptr<Command>& message)
  {
    //std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
    
    cache_.push(message);
    notifyWrite();
  }

  void onMessage(const ConnectionPtr& connection)
  { 
    connection_->loop()->assertInLoopThread();
    Buffer& input = connection->input();
    LOG_TRACE << "read=" << input.toString() ;
    //std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection->getContext());
    while(cache_.readResponse(input))// more response
    {
    }
  }

  void writeRequest()
  {
    connection_->loop()->assertInLoopThread();
    //std::shared_ptr<Context> context = std::static_pointer_cast<Context>(connection_->getContext());
    cache_.writeRequest(connection_);
  }

  void notifyWrite()
  {
    connection_->loop()->runInLoop(std::bind(&ClientImpl::writeRequest, this));
  }
};

AsyncClient::AsyncClient(EventLoop* loop, const char* host, int port, CountDownLatch& latch, int connSize)
  : latch_(latch)
{
  impls.reserve(connSize);
  shared_ptr<ClientImpl> impl(new ClientImpl(loop, host, port, latch));
  impls.push_back(std::move(impl));
};

void AsyncClient::connect()
{
  for (auto itr = impls.begin(); itr != impls.end(); itr++)
    (*itr)->connect();
};

void AsyncClient::disconnect()
{
  for (auto itr = impls.begin(); itr != impls.end(); itr++)
    (*itr)->disconnect();
};

future<AsyncClient::Msg> AsyncClient::sendFuture(Message request)
{
  int index = random(0, impls.size() - 1);
  return impls[index]->sendFuture(request);
};

void AsyncClient::send(const std::shared_ptr<Command>& command)
{ 
  int size = impls.size();
  int index = random(0, size - 1);
  return impls[index]->send(command);
};

int AsyncClient::random(const int& min, const int& max) 
{
  static thread_local std::mt19937 generator;
  std::uniform_int_distribution<int> distribution(min,max);
  return distribution(generator);
};

}
}
