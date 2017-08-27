#include <libnet/timestamp.h>
#include <libnet/logger.h>
#include <libnet/countdown_latch.h>
#include <libnet/mc/session.h>
#include <libnet/mc/req_cache.h>

namespace mc
{
namespace client
{
using namespace libnet;
using namespace std;
using namespace std::placeholders;
// 一旦触发 high_water_mark_ request不能往 cache投递，cache 里面的 request会继续往buffer写，直到buffer写完
Session::Session(EventLoop* loop, 
                 const char* host, 
                 uint16_t port,
                 CountDownLatch& connected_latch,
                 CountDownLatch& closed_latch,
                 size_t high_water_mark,
                 uint32_t idle_timeout_milli,
                 size_t max_retry)
  : loop_(loop),
    client_(loop, host, port),
    cache_(),
    connected_latch_(connected_latch),
    closed_latch_(closed_latch),
    high_water_mark_(high_water_mark),
    reject_write_(false),
    timer_id_(),
    conn_(),
    connected_(false),
    last_optime_(0),
    idle_timeout_milli_(idle_timeout_milli),
    max_retry_(max_retry),
    retries_(0)
{
  
};

Session::Session(EventLoop* loop, 
                 const InetAddress& remote_addr,
                 CountDownLatch& connected_latch,
                 CountDownLatch& closed_latch,
                 size_t high_water_mark,
                 uint32_t idle_timeout_milli,
                 size_t max_retry)
  : loop_(loop),
    client_(loop, remote_addr),
    cache_(),
    connected_latch_(connected_latch),
    closed_latch_(closed_latch),
    high_water_mark_(high_water_mark),
    reject_write_(false),
    timer_id_(),
    conn_(),
    connected_(false),
    last_optime_(0),
    idle_timeout_milli_(idle_timeout_milli),
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
    if (idle_timeout_milli_ > 0)
    {
      timer_id_ = loop_->runAfter(idle_timeout_milli_, std::bind(&Session::onIdle, this));
      update();
    }
    conn->setHighWaterMarkCallBack(std::bind(&Session::rejectWrite, this, _1, _2), high_water_mark_);
    conn->setWriteCompleteCallBack(std::bind(&Session::acceptWrite, this, _1));
    connected_ = true;
    cache_.start();
    connected_latch_.countDown();
    closed_latch_.add();
  }
  else if (conn->disconnected())
  {
    connected_ = false;
    conn_.reset();
    if (idle_timeout_milli_ > 0 && timer_id_)
    {
     loop_->cancel(timer_id_);
    }
    cache_.close();
    closed_latch_.countDown();
  }
};

bool Session::send(const std::shared_ptr<Command>& cmd, int32_t send_wait_milli, bool check_cache_reject) // check connected?
{    
  if (!cache_.send(cmd, send_wait_milli, check_cache_reject))
  {
    LOG_TRACE << "send_wait_milli = "
              << send_wait_milli
              << " check_cache_reject = "
              << check_cache_reject
              << " result = false" ;
    return false;
  }
  notifyWrite();
  return true;
};

void Session::onMessage(const Conn& conn)
{ 
  loop_->assertInLoopThread();
  Buffer& input = conn->input();
  if (log::Logger::getLogLevel() <= libnet::log::TRACE)
    LOG_TRACE << "read = " << input.toString() ;
  update();
  while(cache_.receive(input))// more response
  {

  }
};

void Session::writeInLoop()
{
  loop_->assertInLoopThread();
  if (conn_ && conn_->connected())
  {
    size_t remain = cache_.write(conn_);
    if (remain > 0)
    {
      loop_->runInLoop(std::bind(&Session::writeInLoop, this));
    }
  }
};

void Session::notifyWrite()
{
  if (reject_write_) return;
  loop_->runInLoop(std::bind(&Session::writeInLoop, this));
};

void Session::onIdle()
{
  loop_->assertInLoopThread();
  if (!conn_ || !conn_->connected()) return;
  uint64_t now = Timestamp::now().milliSecondsValue();
  int duration = now - last_optime_;

  if (duration < idle_timeout_milli_)
  {
    LOG_TRACE << " runAfter " << (idle_timeout_milli_ - duration);
    timer_id_ = loop_->runAfter(idle_timeout_milli_ - duration, std::bind(&Session::onIdle, this));
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
      send(cmd, 0, false);
      LOG_TRACE << " retries = " << retries_ << " runAfter " << (idle_timeout_milli_);
      timer_id_ = loop_->runAfter(idle_timeout_milli_, std::bind(&Session::onIdle, this));
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

void Session::acceptWrite(const Conn& conn)
{
  loop_->assertInLoopThread();
  //cache_.acceptSending();
  reject_write_ = false;
  notifyWrite();
};

void Session::rejectWrite(const Conn& conn, size_t size)
{
  loop_->assertInLoopThread();
  reject_write_ = true;
  //cache_.rejectSending();
};

}
}
