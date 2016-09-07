#include "connector.h"
#include "eventloop.h"
#include "channel.h"
#include "socket_ops.h"

namespace libnet
{

Connector::Connector(EventLoop* loop, const InetAddress& server_address)
  : stop_(false),
    state_(kDisConnected),
    loop_(loop),
    server_address_(server_address),
    channel_(),
    lock_()
{

};

Connector::~Connector()
{
  stop_ = true;
  state_ = kDisConnected;
  LockGuard guard(lock_);
  if (channel_)
  {
    channel_->disableAll();
    channel_->remove();
    LOG_DEBUG << "close" ;
    sockets::close(channel_->fd());
  }
};

void Connector::start()
{
  stop_ = false;
  loop_->runInLoop(std::bind(&Connector::startInLoop, this));
};

void Connector::restart()
{
  loop_->assertInLoopThread();
  stop_ = false;
  state_ = kDisConnected;
  startInLoop();
};

void Connector::startInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kDisConnected)
  {
    if (!stop_)
    {
      connectInLoop();//本次调用设置 kConnecting 不会同时发起多个
    }
    else
    {
      LOG_DEBUG << "stopped!";
    }
  }
};

void Connector::stop()
{
  stop_ = true;
  loop_->runInLoop(std::bind(&Connector::stopInLoop, this));
};

void Connector::stopInLoop()
{
  loop_->assertInLoopThread();
  if (state_ == kConnecting)
  {
    state_ = kDisConnected;
    LockGuard guard(lock_);
    channel_->disableAll();
    channel_->remove();
    loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this()));
  }
};


void Connector::registerConnect(int fd)
{
  loop_->assertInLoopThread();
  state_ = kConnecting;
  LOG_DEBUG << "fd=" << fd ; 
  LockGuard guard(lock_);
  channel_.reset(new Channel(loop_, fd));
  channel_->setWriteCallback(std::bind(&Connector::handleWrite, this, fd));
  channel_->setErrorCallback(std::bind(&Connector::handleError, this, fd));
  channel_->enableWriting();  //error ?
};

void Connector::connectInLoop()
{

  //if (stop_) return;
  loop_->assertInLoopThread();
  int fd = sockets::createSocketFd();
  sockets::setNoBlocking(fd);
  int r = sockets::connect(fd, server_address_.getSockAddrIn());
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
  loop_->runAfter(2000, std::bind(&Connector::startInLoop, shared_from_this()));
};

void Connector::handleWrite(int fd)
{
  loop_->assertInLoopThread();
  LOG_DEBUG << "fd=" << fd ;
  LockGuard guard(lock_);
  channel_->disableAll();
  channel_->remove();

  int err = sockets::getSocketError(fd);//the fd is readable and writeable when error
  
  if (err)
  {
    sockets::close(fd); 
    // handleWrite is excuted inside the Channle::handleEvent
    loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this()));
    retry();//removeChannelInLoop() excute before the startInLoop() in retry()
    return;
  }

  if (!stop_ && new_connection_callback_)
  {
    state_ = kConnected;
    loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this()));
    new_connection_callback_(fd);
  }
  else
  {
    state_ = kDisConnected;
    sockets::close(fd);
    loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this()));
  }
};

void Connector::handleError(int fd)
{  
  loop_->assertInLoopThread();
  LOG_DEBUG << "fd=" << fd ;
  LockGuard guard(lock_);
  channel_->disableAll();
  channel_->remove();
  sockets::close(fd);
  loop_->queueInLoop(std::bind(&Connector::removeChannelInLoop, shared_from_this()));
  retry();
};

void Connector::removeChannelInLoop()
{
  loop_->assertInLoopThread();
  LockGuard guard(lock_);
  channel_.reset();
};
}