#include "connector.h"
#include "eventloop.h"
#include "channel.h"
#include "socket_ops.h"

namespace libnet
{

Connector::Connector(EventLoop* loop, const InetAddress& serverAddress)
  : stop_(false),
    state_(kDisConnected),
    loop_(loop),
    serverAddress_(serverAddress),
    channel_(),
    lock_()
{

};

Connector::~Connector()
{
  //loop_->assertInLoopThread();
  stop_ = true;
  state_ = kDisConnected;
  LockGuard guard(lock_);
  // for (Channels::iterator itr = channels_.begin(); itr != channels_.end(); itr++) 
  // {
  if (channel_)
  {
    channel_->disableAll();
    channel_->remove();
    LOG_DEBUG << "close" ;
    sockets::close(channel_->fd());
  }
  //}
};

void Connector::start()
{
  stop_ = false;
  loop_->runInLoop(std::bind(&Connector::connect, this));
};

void Connector::stop()
{
  stop_ = true;
};

void Connector::connect()
{
  loop_->assertInLoopThread();
  if (state_ == kConnecting) return;
  state_ = kConnecting;
  loop_->runInLoop(std::bind(&Connector::connectInLoop, this));
};

void Connector::registerConnect(int fd)
{
  loop_->assertInLoopThread();

  LOG_DEBUG << "fd=" << fd ; 
  LockGuard guard(lock_);
  channel_.reset(new Channel(loop_, fd));
  channel_->setWriteCallback(std::bind(&Connector::handleWrite, this, fd));
  channel_->setErrorCallback(std::bind(&Connector::handleError, this, fd));
  channel_->enableWriting();  //error ?
};

void Connector::connectInLoop()
{

  if (stop_) return;
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
  channel_->disableAll();
  channel_->remove();

  int err = sockets::getSocketError(fd);//socket 连接建立变为可读，如果出错 即可读又可写
  
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
  channel_->disableAll();
  channel_->remove();
  loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this(), fd, true));
  retry();
};

void Connector::removeChannelInLoop(int fd, bool close)
{
  loop_->assertInLoopThread();
  LockGuard guard(lock_);
  channel_.reset();
  if (close)
  {
    sockets::close(fd); 
  }
  else
  {
    state_ = kConnected;
  }
};
}