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
    state_(CONNECTING),
    channel_(new Channel(loop, fd)),
    socket_(new Socket(fd)),
    
    inputBuffer_(4, 1024),
    outputBuffer_(4, 1024),
    id_(id)

{
  channel_->setReadCallback(std::bind(&Connection::handleRead, this));//std::placeholders::_1
  channel_->setWriteCallback(std::bind(&Connection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&Connection::handleClose, this));//channel callback使用的connection 不不必使用share_ptr ,因为直接disableAll,没机会被调用了
  channel_->setErrorCallback(std::bind(&Connection::handleError, this));
};


void Connection::establish()
{
  LOG_INFO <<"connection Id=" << id_ << ", fd=" << (channel_->fd()) ;
  loop_->assertInLoopThread();
  state_ = CONNECTED;
  if (connectionCallBack_)
    connectionCallBack_(shared_from_this());
  channel_->enableReading();
};

void Connection::destroy()
{
  loop_->assertInLoopThread();
  LOG_INFO <<"connection Id=" << id_ << " state=" << state_ <<" fd=" << (channel_->fd()) ;
  if (state_ == CONNECTED)
  {
    state_ = DIS_CONNECTED;
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
  if (state_ != CONNECTED) return;
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
  if (state_ != CONNECTED) return;
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
  if (state_ != CONNECTED) return;
  if (!loop_->inLoopThread())
  {
    loop_->queueInLoop(std::bind(&Connection::sendBufferInLoop, this, buffer));//send 必须持有 shared_ptr
  }
  else
  {
    sendBufferInLoop(buffer);
  }
};


void Connection::sendStringInLoop(const CString& cstring)
{
  loop_->assertInLoopThread();
  if (state_ == DIS_CONNECTED)
  {
    LOG_ERROR <<"connection Id=" << id_ << ", fd=" << (channel_->fd()) << ", error=disconnected";
    return;
  }
  int n = 0;
  if (outputBuffer_.readable() == 0)
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
    if (state_ == DIS_CONNECTED)
    {
        LOG_ERROR <<"conId-" <<id_ << ", fd-" << (channel_->fd()) << ", error-disconnected";
        return;
    }
    int n = 0;
    if (outputBuffer_.readable() == 0)
    {
        n = socket_->write(*buffer);
    }
    if (n < 0)
    {
        handleError();
    }
    else
    {
        //remain = remain - n;
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
    if (state_ != CONNECTED) return;
    state_ = DIS_CONNECTING;
    loop_->runInLoop(std::bind(&Connection::shutdownInLoop, shared_from_this()));
};

void Connection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_ -> isWriting())//队列中的全部写完
  {
    socket_->shutdownWrite();//写关闭，由handleRead触发handleClose
  }
};

void Connection::handleRead()
{
  loop_->assertInLoopThread();
  //int len = 0;
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
        if (state_ == DIS_CONNECTING)
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
  state_ = DIS_CONNECTED;

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
  if (state_ >= CONNECTING && state_ <= DIS_CONNECTED)
    return state::StateName[state_];
  else return "unknow";
};

Connection::~Connection()
{
  LOG_TRACE << " destroy connection " ; 
};

}