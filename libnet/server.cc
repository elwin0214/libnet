#include <signal.h>
#include <errno.h>
#include "server.h"
#include "eventloop_group.h"
#include "inet_address.h"
#include "acceptor.h"
#include "connection.h"
#include "mutexlock.h"  
#include "logger.h"

namespace libnet
{
namespace signal
{

struct IgnoreSigPipe
{
IgnoreSigPipe()
{
  ::signal(SIGPIPE, SIG_IGN); 
  LOG_TRACE << "errno=" << errno;
}
};

}

signal::IgnoreSigPipe gIgnoreSigPipe;

Server::Server(EventLoop* loop, const char* host, int port, int workersNum)
  : loop_(loop),
    localAddr_(host, port),
    acceptor_(loop_, localAddr_, 100),
    loopGroupPtr_(new EventLoopGroup(loop_, workersNum, "worker")),
    nextConId_(1),
    started_(false)
{
  acceptor_.setNewConnectionCallback(std::bind(&Server::newConnection, this, std::placeholders::_1, std::placeholders::_2));
};

Server::~Server()
{
  loop_->assertInLoopThread();
  for (std::map<int, ConnectionPtr>::iterator itr = connections_.begin();
        itr != connections_.end(); itr++)
  {
    ConnectionPtr conn = itr->second;
    itr->second.reset();
    LOG_TRACE << "connection Id=" << (conn->id()) ;
    conn->loop()->runInLoop(std::bind(&Connection::destroy, conn));//this?
    conn.reset();
  }
};

void Server::start()
{
  if (started_) return;
  started_ = true;
  loopGroupPtr_->start();
  acceptor_.start();
  LOG_DEBUG <<" server start...";
};

// run in acceptor loop
void Server::newConnection(int fd, InetAddress& addr)
{
  loop_->assertInLoopThread();
  EventLoop* loopPtr = loopGroupPtr_->getNextLoop();
  int id = nextConId_++;
  ConnectionPtr connPtr = std::make_shared<Connection>(loopPtr, fd, id);
  connPtr->setConnectionCallBack(connectionCallBack_);
  connPtr->setReadCallBack(messageCallBack_);
  connPtr->setCloseCallBack(std::bind(&Server::removeConnection, this, std::placeholders::_1));  // server 关闭时，removeConnectionInLoop 执行？
  connections_[id] = connPtr;
  loopPtr->runInLoop(std::bind(&Connection::establish, connPtr));  
};

void Server::removeConnection(const ConnectionPtr &connPtr)
{
  loop_->runInLoop(std::bind(&Server::removeConnectionInLoop, this, connPtr)); 
};

void Server::removeConnectionInLoop(const ConnectionPtr &connPtr) 
{
  loop_->assertInLoopThread();
  int id = connPtr->id();
  LOG_DEBUG << "connection id=" << id ;
  connections_.erase(id);
  connPtr->loop()->queueInLoop(std::bind(&Connection::destroy, connPtr));// 最后在worker 里面destroy
};


}