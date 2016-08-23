#include <assert.h>
#include <functional> 
#include <errno.h>
#include "connection.h"
#include "channel.h"
#include "socket.h"
#include "logger.h"
#include "eventloop.h"

namespace libnet
{

namespace state
{
const char* StateName[4] = 
{
  "CONNECTING", 
  "CONNECTED", 
  "DIS_CONNECTING", 
  "DIS_CONNECTED"
};
}

using namespace std::placeholders;
Connection::Connection(EventLoop* loop, int fd, /*InetAddress &addr, */int id)
  : loop_(loop),
    state_(kConnecting),
    channel_(new Channel(loop, fd)),
    socket_(new Socket(fd)),
    inputBuffer_(4, 1024),
    outputBuffer_(4, 1024),
    id_(id)

{
  channel_->setReadCallback(std::bind(&Connection::handleRead, this));//std::placeholders::_1
  channel_->setWriteCallback(std::bind(&Connection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&Connection::handleClose, this));//channel callback使用的connection 不必使用share_ptr ,因为直接disableAll,没机会被调用了
  channel_->setErrorCallback(std::bind(&Connection::handleError, this));
};


void Connection::establish()
{
  LOG_INFO <<"connection Id = " << id_ << ", fd = " << (channel_->fd()) ;
  loop_->assertInLoopThread();
  state_ = kConnected;
  if (connectionCallBack_)
    connectionCallBack_(shared_from_this());
  channel_->enableReading();
};

void Connection::destroy()
{
  loop_->assertInLoopThread();
  LOG_INFO <<" connection Id = " << id_ << " state = " << state_ <<" fd = " << (channel_->fd()) ;
  if (state_ == kConnected)
  {
    state_ = kDisConnected;
    channel_->disableAll();
    if (connectionCallBack_)
    {
      connectionCallBack_(shared_from_this());
    }
  }
  channel_->remove();
};
void Connection::sendString(const CString& cstring)
{
  if (state_ != kConnected) return;
  if (!loop_->inLoopThread())
  {
    loop_->queueInLoop(std::bind(&Connection::sendStringInLoop, shared_from_this(), cstring));
  }
  else
  {
    sendStringInLoop(cstring);
  }
};

void Connection::send(const std::shared_ptr<Buffer>& buffer)
{
  if (state_ != kConnected) return;
  if (!loop_->inLoopThread())
  {
    loop_->queueInLoop(std::bind(&Connection::sendInLoop, shared_from_this(), buffer));//send 必须持有 shared_ptr
  }
  else
  {
    sendInLoop(buffer);
  }
};

void Connection::sendBuffer(Buffer* buffer)
{
  if (state_ != kConnected){
    delete buffer;
    return;
  } 
  if (!loop_->inLoopThread())
  {
    loop_->queueInLoop(std::bind(&Connection::sendBufferInLoop, shared_from_this(), buffer));//send 必须持有 shared_ptr
  }
  else
  {
    sendBufferInLoop(buffer);
  }
};


void Connection::sendStringInLoop(const CString& cstring)
{
  loop_->assertInLoopThread();
  if (state_ == kDisConnected)
  {
    LOG_ERROR <<"connection Id=" << id_ << ", fd=" << (channel_->fd()) << ", error=disconnected";
    return;
  }
  int n = 0;
  if (!channel_->isWriting() && outputBuffer_.readable() == 0)
  {
    //LOG_
    n = socket_->write(cstring);
  }
  if (n < 0)
  {
    handleError();
  }
  else
  {
    if (n < cstring.length())
    {
      outputBuffer_.append(cstring, n);
      channel_->enableWriting();
    }
  }
};

void Connection::sendInLoop(const std::shared_ptr<Buffer>& buffer)
{
  loop_->assertInLoopThread();
  if (state_ == kDisConnected)
  {
    LOG_ERROR <<"conId-" <<id_ << ", fd-" << (channel_->fd()) << ", error-disconnected";
    return;
  }
  int n = 0;
  if (!channel_->isWriting() && outputBuffer_.readable() == 0)
  {
    n = socket_->write(*buffer);
  }
  if (n < 0)
  {
    handleError();
  }
  else
  {
    int remain = buffer->readable();
    if (remain > 0)
    {
      outputBuffer_.append(*buffer);
      channel_->enableWriting();
    }
  }
};

void Connection::sendBufferInLoop(Buffer* buffer)
{
  loop_->assertInLoopThread();
  sendStringInLoop(CString(buffer->beginRead(), buffer->readable()));
  delete buffer;
};

void Connection::shutdown()
{
    if (state_ != kConnected) return;
    state_ = kDisConnecting;
    loop_->runInLoop(std::bind(&Connection::shutdownInLoop, shared_from_this()));
};

void Connection::shutdownInLoop()//当发起shutdown 最后执行时，前面已经提交的send全部发送完
{
  loop_->assertInLoopThread();
  if (!channel_ -> isWriting())//还有数据需要写，由handleWrite发起关闭
  {
    socket_->shutdownWrite();//写关闭，由handleRead触发handleClose
  }
};

void Connection::handleRead()
{
  loop_->assertInLoopThread();
  int n = socket_->read(inputBuffer_);
  if (n == 0)
  {
    handleClose();
  }
  else if(n < 0)
  {
    if ((n == EAGAIN || n == EWOULDBLOCK))
    {
        
    }
    else
    {
      LOG_SYSERROR << "handleRead!" ;
      handleError();    
    }
  }
  else
  {
    if(readCallBack_)
      readCallBack_(shared_from_this());
  }
};

void Connection::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    int n = socket_->write(outputBuffer_);
    if (n >= 0)
    {
      if(outputBuffer_.readable() == 0)
      {
        channel_->disableWriting();
        if (state_ == kDisConnecting)
        {
          shutdownInLoop();
        }
      }
    }
    else
    {
      handleError();
    }
  }
};

void Connection::handleClose()
{
  loop_->assertInLoopThread();
  state_ = kDisConnected;

  channel_->disableAll();

  if (connectionCallBack_)
  {
    connectionCallBack_(shared_from_this());  // 这里必须用 shared_ptr
  }
  if (closeCallBack_)
  {
    closeCallBack_(shared_from_this());
  }
};

void Connection::handleError()
{
  loop_->assertInLoopThread();
  if (connectionCallBack_)
  {
    
  }
};
const char* Connection::stateToString()
{
  if (state_ >= kConnecting && state_ <= kDisConnected)
    return state::StateName[state_];
  else return "unknow";
};

Connection::~Connection()
{
  LOG_TRACE << " destroy connection " ; 
};

}