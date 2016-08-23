#include "client.h"
#include "connection.h"
#include "channel.h"
#include "logger.h"

namespace libnet
{

Client::Client(EventLoop* loop, const char* host, int port)
  : loop_(loop),
    serverAddr_(host, port),
    connector_(std::make_shared<Connector>(loop_, serverAddr_)),
    connId_(1),
    connectionPtr_()
{
  connector_->setNewConnectionCallBack(std::bind(&Client::newConnection, this, std::placeholders::_1));
};

Client::~Client()
{
  LOG_DEBUG << "~Client()" ;
  // for (std::map<int, ConnectionPtr>::iterator itr = connections_.begin();
  //       itr != connections_.end(); itr++)
  // {
  //   ConnectionPtr conn = itr->second;
  //   itr->second.reset();
  //   conn->loop()->runInLoop(std::bind(&Connection::destroy, conn));
  //   conn.reset();
  // }
  if (connectionPtr_)
  {
    loop_->runInLoop(std::bind(&Connection::destroy, connectionPtr_));
    connectionPtr_.reset();
  }
};

void Client::connect()
{
  connector_->start();
  // for (int i = 0; i < connNum_; i++)
  // {
  //   connector_->connect();
  // }
};

void Client::disconnect()
{
  connector_->stop();
  loop_->runInLoop(std::bind(&Client::disconnectInLoop, this));
};

void Client::disconnectInLoop()
{
  loop_->assertInLoopThread();
  if (connectionPtr_)
  {
    loop_->runInLoop(std::bind(&Connection::shutdown, connectionPtr_));
    //connectionPtr_->reset();
  }
  
  // for (std::map<int, ConnectionPtr>::iterator itr = connections_.begin();
  //       itr != connections_.end(); itr++)
  // {
  //   ConnectionPtr conn = itr->second;
  //   itr->second.reset();
  //   conn->loop()->runInLoop(std::bind(&Connection::shutdown, conn));
  //   conn.reset();
  // }
};

void Client::newConnection(int fd)
{
  loop_->assertInLoopThread();
  ConnectionPtr connectionPtr = std::make_shared<Connection>(loop_, fd, connId_++);
  connectionPtr->setConnectionCallBack(connectionCallBack_);
  connectionPtr->setReadCallBack(messageCallBack_);
  connectionPtr->setCloseCallBack(std::bind(&Client::removeConnection, this, std::placeholders::_1));
  connectionPtr_ = connectionPtr;
  connectionPtr->loop()->runInLoop(std::bind(&Connection::establish, connectionPtr));
};

void Client::removeConnection(const ConnectionPtr& connPtr)
{
  //loop_->runInLoop(std::bind(&Client::removeConnectionInLoop, this, connPtr));
  loop_->queueInLoop(std::bind(&Connection::destroy, connPtr));
  connectionPtr_.reset();

};

// void Client::removeConnectionInLoop(const ConnectionPtr &connPtr) 
// {
//   loop_->assertInLoopThread();
//   int id = connPtr->id();
//   LOG_DEBUG << "connection id=" << id ;
//   connections_.erase(id);
//   connPtr->loop()->queueInLoop(std::bind(&Connection::destroy, connPtr));// 最后在worker 里面destroy
// };
}
