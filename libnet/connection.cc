#include <assert.h>
#include <functional> 
#include <errno.h>
#include <libnet/connection.h>
#include <libnet/channel.h>
#include <libnet/socket.h>
#include <libnet/logger.h>
#include <libnet/eventloop.h>

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
    input_(4, 1024),
    output_(4, 1024),
    id_(id),
    name_()

{
  channel_->setReadCallback(std::bind(&Connection::handleRead, this));//std::placeholders::_1
  channel_->setWriteCallback(std::bind(&Connection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&Connection::handleClose, this));//channel callback使用的connection 不必使用share_ptr ,因为直接disableAll,没机会被调用了
  channel_->setErrorCallback(std::bind(&Connection::handleError, this));
  socket_->setNoBlocking();
  socket_->setKeepAlive(true);
};

void Connection::establish()
{
  LOG_INFO <<"connection Id = " << id_ << ", fd = " << (channel_->fd()) ;
  loop_->assertInLoopThread();
  state_ = kConnected;
  if (connection_callback_)
    connection_callback_(shared_from_this());
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
    if (connection_callback_)
    {
      connection_callback_(shared_from_this());
    }
  }
  channel_->remove();
};

void Connection::send(const CString& cstring)
{
  if (state_ != kConnected) return;
  if (!loop_->inLoopThread())
  {
    loop_->queueInLoop(std::bind(&Connection::sendInLoop, shared_from_this(), cstring.toString()));
  }
  else
  {
    sendInLoop(cstring);
  }
};

void Connection::sendInLoop(const CString& cstring)
{
  loop_->assertInLoopThread();
  if (state_ == kDisConnected)
  {
    LOG_ERROR <<"connection Id = " << id_ << " fd = " << (channel_->fd()) << " error = disconnected";
    return;
  }
  size_t before = output_.readable();
  int n = 0;
  if (!channel_->isWriting() && output_.readable() == 0)
  {
    n = socket_->write(cstring);
    if (n == cstring.length() && write_complete_callback_)
      loop_->queueInLoop(std::bind(write_complete_callback_, shared_from_this()));
  }
  if (n < 0)
  {
    handleError();
  }
  else
  {
    if (n < cstring.length())
    {
      output_.append(cstring, n);
      channel_->enableWriting();
    }
  }
  size_t after = output_.readable();
  if (before <= high_water_mark_
      && after > high_water_mark_
      && high_watermark_callback_)
  {
    loop_->queueInLoop(std::bind(high_watermark_callback_, shared_from_this(), after));
  }
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
  int n = socket_->read(input_);
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
    if(read_callback_)
      read_callback_(shared_from_this());
  }
};

void Connection::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    int n = socket_->write(output_);
    if (n >= 0)
    {
      if(output_.readable() == 0)
      {
        channel_->disableWriting();
        if (write_complete_callback_)
          loop_->queueInLoop(std::bind(write_complete_callback_, shared_from_this()));
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

void Connection::handleClose()// 内部触发
{
  loop_->assertInLoopThread();
  state_ = kDisConnected;

  channel_->disableAll();

  if (connection_callback_)
  {
    connection_callback_(shared_from_this()); // 先 connection_callback_ 再 close_callback_ 
  }
  if (close_callback_)
  {
    close_callback_(shared_from_this());// 外部注入的调用，connection关闭时候触发，会把最后一个share_ptr 放入loop 尾部
  }
 };

void Connection::handleError()
{
  loop_->assertInLoopThread();

  int err = sockets::getSocketError(socket_->fd());
  if (err & (ETIMEDOUT | EHOSTUNREACH | ENETUNREACH | EPIPE | ECONNRESET))
  {
    LOG_ERROR <<"fd = "<< socket_->fd() <<" err = " << err << " error = " << log::Error(err);
    if (state_ != kDisConnected)
      handleClose();
  }
};

void Connection::setTcpNoDelay(bool on)
{
  socket_->setTcpNoDelay(on);
};

void Connection::enableReading()
{
  loop_->runInLoop(std::bind(&Channel::enableReading, channel_));
};

void Connection::disableReading()
{
  loop_->runInLoop(std::bind(&Channel::disableReading, channel_));
};

const char* Connection::stateToString()
{
  int state = state_.load();
  if (state >= kConnecting && state <= kDisConnected)
    return state::StateName[state];
  else return "unknow";
};

Connection::~Connection()
{
  LOG_TRACE << " destroy connection " ; 
};

}