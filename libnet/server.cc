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

Server::Server(EventLoop* loop, const char* host, int port, int workers)
  : loop_(loop),
    local_addr_(host, port),
    acceptor_(loop_, local_addr_, 100),
    loop_group_(new EventLoopGroup(loop_, workers, "worker")),
    next_id_(1),
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
    LOG_DEBUG << "connection =" << (conn->get_name()) ;
    conn->loop()->runInLoop(std::bind(&Connection::destroy, conn));//this?
    conn.reset();
  }
};

void Server::start()
{
  if (started_) return;
  started_ = true;
  loop_group_->start();
  acceptor_.start();
  LOG_DEBUG <<" server start...";
};

// run in acceptor loop
void Server::newConnection(int fd, InetAddress& addr)
{
  loop_->assertInLoopThread();
  EventLoop* loop = loop_group_->getNextLoop();
  int id = next_id_++;
  ConnectionPtr connection = std::make_shared<Connection>(loop, fd, id);
  connection->setConnectionCallBack(connection_callback_);
  connection->setReadCallBack(message_callback_);
  connection->setCloseCallBack(std::bind(&Server::removeConnection, this, std::placeholders::_1));  // server 关闭时，removeConnectionInLoop 执行？
  connections_[id] = connection;
  std::string name;
  name.reserve(32);
  name.append(std::to_string(id))
      .append("-")
      .append(addr.getIp())
      .append(":")
      .append(std::to_string(addr.getPort()));
  LOG_DEBUG << "connection = " << name ;
  connection->set_name(std::move(name));
  loop->runInLoop(std::bind(&Connection::establish, connection));  
};

void Server::removeConnection(const ConnectionPtr &connection)//2次操作，避免锁竞争
{
  loop_->runInLoop(std::bind(&Server::removeConnectionInLoop, this, connection)); 
};

void Server::removeConnectionInLoop(const ConnectionPtr &connection) 
{
  loop_->assertInLoopThread();
  int id = connection->id();
  LOG_DEBUG << "connection =" << (connection->get_name());
  connections_.erase(id);
  connection->loop()->queueInLoop(std::bind(&Connection::destroy, connection));// 最后在worker 里面destroy
};


}