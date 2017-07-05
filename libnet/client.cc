#include "client.h"
#include "connection.h"
#include "channel.h"
#include "logger.h"

namespace libnet
{

Client::Client(EventLoop* loop, const char* host, int port)
  : loop_(loop),
    server_addr_(host, port),
    connector_(std::make_shared<Connector>(loop_, server_addr_)),
    next_id_(1),
    connection_(),
    stop_(false),
    retry_(false),
    lock_()
{
  connector_->setNewConnectionCallBack(std::bind(&Client::newConnection, this, std::placeholders::_1));
};

Client::~Client()
{
  LOG_DEBUG << "~Client()" ;
  LockGuard guard(lock_);
  if (connection_) //a
  {
    LOG_DEBUG << "~Client runInLoop b " << connection_.use_count() ;
    loop_->runInLoop(std::bind(&Connection::destroy, connection_));
    LOG_DEBUG << "~Client runInLoop a " << connection_.use_count() ;
    connection_.reset();
  }
};

void Client::connect()
{
  stop_ = false;
  connector_->start();
};

void Client::disconnect()
{
  stop_ = true;
  connector_->stop();
  loop_->runInLoop(std::bind(&Client::disconnectInLoop, this));
};

void Client::disconnectInLoop()
{
  loop_->assertInLoopThread();
  if (connection_)
  {
    loop_->runInLoop(std::bind(&Connection::shutdown, connection_));
  }
};

void Client::newConnection(int fd)
{
  loop_->assertInLoopThread();
  ConnectionPtr connection = std::make_shared<Connection>(loop_, fd, next_id_++);
  connection->setConnectionCallBack(connection_callback_);
  connection->setReadCallBack(message_callback_);
  connection->setCloseCallBack(std::bind(&Client::removeConnection, this, std::placeholders::_1));
  {
    LockGuard guard(lock_);
    connection_ = connection;
  }
  connection->loop()->runInLoop(std::bind(&Connection::establish, connection_));
};

void Client::removeConnection(const ConnectionPtr& connection)
{
  LOG_TRACE << "removeConnection " << connection.use_count();
  LockGuard guard(lock_);
  if (connection_) //b 与a处 竞争，有可能2处都会调用，因为外部还存在shared_ptr
  {
    LOG_TRACE << "removeConnection b " << connection_.use_count() ;
    loop_->queueInLoop(std::bind(&Connection::destroy, connection));
    LOG_TRACE << "removeConnection a " << connection_.use_count() ;
    connection_.reset();
  }

  if (!stop_ && retry_)
  {
    connector_->restart();
  }
};

}
