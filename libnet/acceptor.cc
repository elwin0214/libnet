#include <errno.h>
#include <functional>
#include <libnet/acceptor.h>
#include <libnet/socket_ops.h>
#include <libnet/socket.h>
#include <libnet/channel.h>
#include <libnet/eventloop.h>
#include <libnet/logger.h>

namespace libnet
{

Acceptor::Acceptor(EventLoop *loop, const InetAddress& addr, int backlog)
  : backlog_(backlog),
    closed_(false),
    loop_(loop),
    socket_(sockets::createSocketFd()),
    channel_(new Channel(loop_, socket_.fd()))
{
  socket_.setReuseAddr();
  socket_.setNoBlocking();
  socket_.bind(addr);
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
  InetAddress peer_addr;
  int fd = socket_.accept(&peer_addr);
  LOG_INFO << "accept fd = " << fd;
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
    if (!closed_ && new_conn_callback_)
    {
      new_conn_callback_(fd, peer_addr);
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
 