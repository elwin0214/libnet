#include <libnet/client.h>
#include <libnet/connection.h>
#include <libnet/channel.h>
#include <libnet/logger.h>

namespace libnet
{

Client::Client(EventLoop* loop, const char* host, uint16_t port)
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

Client::Client(EventLoop* loop, const InetAddress& address)
  : loop_(loop),
    server_addr_(address),
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
  Conn conn = std::make_shared<Connection>(loop_, fd, next_id_++);
  conn->setConnectionCallBack(connection_callback_);
  conn->setReadCallBack(message_callback_);
  conn->setCloseCallBack(std::bind(&Client::removeConnection, this, std::placeholders::_1));
  {
    LockGuard guard(lock_);
    connection_ = conn;
  }
  loop_->runInLoop(std::bind(&Connection::establish, connection_));
};

void Client::removeConnection(const Conn& conn)
{
  LockGuard guard(lock_);
  if (connection_) //b 与a处 竞争，有可能2处都会调用，因为外部还存在shared_ptr
  {
    loop_->queueInLoop(std::bind(&Connection::destroy, conn));
    LOG_TRACE << "removeConnection " << connection_.use_count() ;
    connection_.reset();
  }

  if (!stop_ && retry_)
  {
    connector_->restart();
  }
};

}
