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
  if (connection_)
  {
    loop_->runInLoop(std::bind(&Connection::destroy, connection_));
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
  loop_->queueInLoop(std::bind(&Connection::destroy, connection));
  LockGuard guard(lock_);
  connection_.reset();
  if (!stop_ && retry_)
  {
    connector_->restart();
  }
};

}
