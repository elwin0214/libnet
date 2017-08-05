#include <libnet/timestamp.h>
#include <libnet/logger.h>
#include <libnet/countdown_latch.h>
#include <libnet/mc/session.h>
#include "req_cache.cc"

namespace mc
{
namespace client
{
using namespace libnet;
using namespace std;

Session::Session(EventLoop* loop, 
                 const char* host, 
                 uint16_t port,
                 CountDownLatch& connected_latch,
                 CountDownLatch& closed_latch,
                 uint32_t idle_timeout,
                 size_t max_retry)
  : loop_(loop),
    client_(loop, host, port),
    cache_(make_shared<RequestCache>()),
    connected_latch_(connected_latch),
    closed_latch_(closed_latch),
    timer_id_(),
    conn_(),
    connected_(false),
    last_optime_(0),
    idle_timeout_(idle_timeout),
    max_retry_(max_retry),
    retries_(0)
{
  
};

Session::Session(EventLoop* loop, 
                 const InetAddress& remote_addr,
                 CountDownLatch& connected_latch,
                 CountDownLatch& closed_latch,
                 uint32_t idle_timeout,
                 size_t max_retry)
  : client_(loop, remote_addr),
    cache_(make_shared<RequestCache>()),
    connected_latch_(connected_latch),
    closed_latch_(closed_latch),
    timer_id_(),
    conn_(),
    connected_(false),
    last_optime_(0),
    idle_timeout_(idle_timeout),
    max_retry_(max_retry),
    retries_(0)
{
  
};

void Session::connect()
{
  client_.setConnectionCallBack(std::bind(&Session::onConnection, this, std::placeholders::_1));
  client_.setMessageCallBack(std::bind(&Session::onMessage, this, std::placeholders::_1));
  client_.connect();
};

void Session::disconnect()
{ 
  client_.disconnect(); 
};

void Session::onConnection(const Conn& conn)
{
  loop_->assertInLoopThread();
  if (conn->connected())
  {
    conn_ = conn;
    if (idle_timeout_ > 0)
    {
      timer_id_ = loop_->runAfter(idle_timeout_, std::bind(&Session::onIdle, this));
      update();
    }
    connected_ = true;
    connected_latch_.countDown();
    closed_latch_.add();
  }
  else if (conn->disconnected())
  {
    connected_ = false;
    conn_.reset();
    if (idle_timeout_ > 0 && timer_id_)
    {
     loop_->cancel(timer_id_);
    }
    cache_->clear();
    closed_latch_.countDown();
  }
};

void Session::send(const std::shared_ptr<Command>& cmd)
{ 
  cache_->push(cmd);
  notifyWrite();
};

void Session::onMessage(const Conn& conn)
{ 
  loop_->assertInLoopThread();
  //std::shared_ptr<Cache> cache = std::static_pointer_cast<Cache>(conn_->getContext());
  Buffer& input = conn->input();
  if (log::Logger::getLogLevel() <= libnet::log::TRACE)
    LOG_TRACE << "read = " << input.toString() ;
  update();
  while(cache_->readResponse(input))// more response
  {

  }
};

void Session::writeRequest()
{
  loop_->assertInLoopThread();
  if (conn_ && conn_->connected())
    cache_->writeRequest(conn_);
};

void Session::notifyWrite()
{
  loop_->runInLoop(std::bind(&Session::writeRequest, this));
};

void Session::onIdle()
{
  loop_->assertInLoopThread();
  if (!conn_ || !conn_->connected()) return;
  uint64_t now = Timestamp::now().milliSecondsValue();
  int duration = now - last_optime_;

  if (duration < idle_timeout_)
  {
    LOG_TRACE << " runAfter " << (idle_timeout_ - duration);
    timer_id_ = loop_->runAfter(idle_timeout_ - duration, std::bind(&Session::onIdle, this));
  }
  else
  {
    if (retries_ < max_retry_)
    {
      retries_++;
      Session* self = this;
      auto callback = [self](const Msg& msg){ 
        if (msg->code() == kSucc) self->update();
      };
      shared_ptr<Command> cmd = make_shared<Command>(Message(kVer), callback);
      send(cmd);
      LOG_TRACE << " runAfter " << (idle_timeout_);
      timer_id_ = loop_->runAfter(idle_timeout_, std::bind(&Session::onIdle, this));
    }
    else
    {
      conn_->shutdown();// 必定存在
    }    
  }
};

void Session::update()
{
  last_optime_ = Timestamp::now().milliSecondsValue();
  retries_ = 0;
};

}
}
