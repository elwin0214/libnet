#include <errno.h>
#include <functional>
#include "acceptor.h"
#include "socket_ops.h"
#include "socket.h"
#include "channel.h"
#include "eventloop.h"
#include "logger.h"


namespace libnet
{

Acceptor::Acceptor(EventLoop *loop, InetAddress& listenAddr, int backlog)
  : backlog_(backlog),
    closed_(false),
    loop_(loop),
    socket_(sockets::createSocketFd()),
    channel_(new Channel(loop_, socket_.fd()))
{
  socket_.setReuseAddr();
  socket_.setNoBlocking();
  socket_.bind(listenAddr);
  channel_->setReadCallback(std::bind(&Acceptor::handleRead, this));

};

void Acceptor::start()
{
  loop_->runInLoop(std::bind(&Acceptor::listen, this));
};

void Acceptor::listen()
{   
  loop_->assertInLoopThread();
  socket_.listen(backlog_);
  channel_->enableReading();
};

void Acceptor::close()
{
  if (closed_) 
    return;
  closed_ = true;
  loop_->runInLoop(std::bind(&Acceptor::handleClose, this));
  loop_->wakeup();
};

void Acceptor::handleClose()
{
  loop_->assertInLoopThread();
  channel_->disableAll();
  channel_->remove();
};

void Acceptor::handleRead()
{
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  int fd = socket_.accept(&peerAddr);
  LOG_INFO << "accept fd=" << fd;
  if (fd == -1)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
    {
      return;
    }
    else if (errno == EMFILE)
    {
      LOG_SYSERROR << " handleRead!";
    }
    else
    {
      LOG_SYSERROR << " handleRead!";
    }
  }
  else
  {
    if (!closed_ && newConnectionCallback_)
    {
      newConnectionCallback_(fd, peerAddr);
      return;
    }
    sockets::close(fd);
  }
};

Acceptor::~Acceptor()
{
  close();
};

}
 