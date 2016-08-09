#include "connector.h"
#include "eventloop.h"
#include "channel.h"
#include "socket_ops.h"

namespace libnet
{

Connector::Connector(EventLoop* loop, const InetAddress& serverAddress)
  : stop_(false),
    loop_(loop),
    serverAddress_(serverAddress),
    channels_(),
    lock_()
{

};

Connector::~Connector()
{
  //loop_->assertInLoopThread();
  stop_ = true;
  LockGuard guard(lock_);
  for (Channels::iterator itr = channels_.begin(); itr != channels_.end(); itr++) 
  {
    itr->second->disableAll();
    itr->second->remove();
    LOG_DEBUG << "close" ;
    sockets::close(itr->second->fd());
  }
};

void Connector::start()
{
  stop_ = false;
};

void Connector::stop()
{
  stop_ = true;
};

void Connector::connect()
{
  if (stop_) return ;
  loop_->runInLoop(std::bind(&Connector::connectInLoop, this));
};

void Connector::registerConnect(int fd)
{
  loop_->assertInLoopThread();
  LOG_DEBUG << "fd=" << fd ; 
  LockGuard guard(lock_);
  channels_[fd].reset(new Channel(loop_, fd));
  channels_[fd]->setWriteCallback(std::bind(&Connector::handleWrite, this, fd));
  channels_[fd]->setErrorCallback(std::bind(&Connector::handleError, this, fd));
  channels_[fd]->enableWriting();  //error ?
};

void Connector::connectInLoop()
{

  if(stop_) return;
  loop_->assertInLoopThread();
  int fd = sockets::createSocketFd();
  sockets::setNoBlocking(fd);
  int r = sockets::connect(fd, serverAddress_.getSockAddrIn());
  int err = (0 == r) ? 0 : errno;  

  switch (err)
  {
    case 0:
    case EINPROGRESS:
    case EISCONN:
    case EINTR:
      registerConnect(fd);
      break;
    default:
      LOG_SYSERROR << "fd=" << fd << " connectInLoop!";
      sockets::close(fd);
      retry();
      break;
  }
};

void Connector::retry()
{ 
  loop_->assertInLoopThread();
  LOG_DEBUG << "retry" ;
  if (stop_) return ;
  loop_->runAfter(2000, std::bind(&Connector::connectInLoop, shared_from_this()));
};

void Connector::handleWrite(int fd)
{
  loop_->assertInLoopThread();
  LOG_DEBUG << "fd=" << fd ;
  LockGuard guard(lock_);
  channels_[fd]->disableAll();
  channels_[fd]->remove();

  int err = sockets::getSocketError(fd);
  
  if (err)
  {
    loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this(), fd, true));
    retry();
    return;
  }

  if (!stop_ && newConnectionCallBack_)
  {
    loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this(), fd, false));
    newConnectionCallBack_(fd);
  }
  else
  {
    loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this(), fd, true));
  }
};

void Connector::handleError(int fd)
{  
  loop_->assertInLoopThread();
  LOG_DEBUG << "fd=" << fd ;
  LockGuard guard(lock_);
  channels_[fd]->disableAll();
  channels_[fd]->remove();
  loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this(), fd, true));
  retry();
};

void Connector::removeChannelInLoop(int fd, bool close)
{
  loop_->assertInLoopThread();
  LockGuard guard(lock_);
  Channels::iterator itr = channels_.find(fd);
  if (itr == channels_.end()) return;
  if (close)
  {
    sockets::close(fd); 
  }
  channels_.erase(fd);

};
}