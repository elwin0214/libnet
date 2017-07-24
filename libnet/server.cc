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
Server::Server(EventLoop* loop, const char* host, uint16_t port, EventLoopGroup* loop_group)
  : loop_(loop),
    local_addr_(host, port),
    acceptor_(loop_, local_addr_, 128),
    loop_group_(loop_group),
    next_id_(1),
    started_(false)
{
  acceptor_.setNewConnectionCallback(std::bind(&Server::newConnection, this, std::placeholders::_1, std::placeholders::_2));
};

Server::Server(EventLoop* loop, const InetAddress& address, EventLoopGroup* loop_group)
  : loop_(loop),
    local_addr_(address),
    acceptor_(loop_, address, 128),
    loop_group_(loop_group),
    next_id_(1),
    started_(false)
{
  acceptor_.setNewConnectionCallback(std::bind(&Server::newConnection, this, std::placeholders::_1, std::placeholders::_2));
};


Server::~Server()
{
  loop_->assertInLoopThread();
  for (std::map<int, Conn>::iterator itr = conns_.begin();
        itr != conns_.end(); itr++)
  {
    Conn conn = itr->second;
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
  //loop_group_->start();
  acceptor_.start();
  LOG_DEBUG <<" server start...";
};

// run in acceptor loop
void Server::newConnection(int fd, InetAddress& addr)
{
  loop_->assertInLoopThread();

  EventLoop* loop = loop_;
  if (nullptr != loop_group_) loop = loop_group_->getNextLoop();
  int id = next_id_++;
  Conn conn = std::make_shared<Connection>(loop, fd, id);
  conn->setConnectionCallBack(connection_callback_);
  conn->setReadCallBack(message_callback_);
  conn->setCloseCallBack(std::bind(&Server::removeConnection, this, std::placeholders::_1));  // server 关闭时，removeConnectionInLoop 执行？
  conns_[id] = conn;
  std::string name;
  name.reserve(32);
  name.append(std::to_string(id))
      .append("-")
      .append(addr.getIp())
      .append(":")
      .append(std::to_string(addr.getPort()));
  LOG_DEBUG << "conn = " << name ;
  conn->set_name(std::move(name));
  loop->runInLoop(std::bind(&Connection::establish, conn));  
};

void Server::removeConnection(const Conn &conn)//~Server 与 removeConnection并发，~Server()后loop中执行removexxx
{
  loop_->runInLoop(std::bind(&Server::removeConnectionInLoop, this, conn)); 
};

void Server::removeConnectionInLoop(const Conn &conn) 
{
  loop_->assertInLoopThread();
  int id = conn->id();
  LOG_DEBUG << "conn =" << (conn->get_name());
  conns_.erase(id);
  conn->loop()->queueInLoop(std::bind(&Connection::destroy, conn));// 最后在worker 里面destroy
};


}