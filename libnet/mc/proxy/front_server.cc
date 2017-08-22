#include <libnet/eventloop.h>
#include <libnet/eventloop_group.h>
#include <libnet/inet_address.h>
#include <libnet/connection.h>
#include "front_server.h"
namespace mc
{
namespace proxy
{

FrontServer::FrontServer(EventLoop* loop,
                         EventLoopGroup* loop_group,
                         const InetAddress& local_address,
                         size_t unack)
  : server_(loop, local_address, loop_group),
    front_unacks_(),
    lock_(),
    req_codec_(),
    resp_codec_(),
    unack_(unack)
{

}

void FrontServer::start()
{
  server_.setConnectionCallBack(std::bind(&FrontServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&FrontServer::onMessage, this, std::placeholders::_1));
  server_.start();
}

void FrontServer::onConnection(const Conn& cn)
{
  if (cn->connected())
  {
    std::shared_ptr<Message> ctx = std::make_shared<Message>();
    cn->setContext(ctx);
    {
      LockGuard guard(lock_);
      front_unacks_[cn->id()] = 0;
    }
  }
  else
  {
    LockGuard guard(lock_);
    front_unacks_.erase(cn->id());
  }
}

void FrontServer::onMessage(const Conn& cn)
{
  std::shared_ptr<Message> request = std::static_pointer_cast<Message>(cn->getContext());
  Buffer& in = cn->input();
  
  while (true)
  {
    if (!req_codec_.decode(*request, in)) return;
    if (request->stat_.code_ != kSucc)
    {
      cn->send("ERROR\r\n");
      request->reset();
      continue;
    }
    LOG_TRACE << " op = " << request->op()  << " key = "  << request->data_.key_;
    message_callback_(cn, *request);
    request->reset();
    {
      LockGuard guard(lock_);
      size_t& unack = front_unacks_[cn->id()];
      unack++ ;
      if (unack >= unack_) cn->disableReading();
    }
  }
}

void FrontServer::send(const Conn& cn, Message& response)
{
  Buffer buffer(0, response.data_.value_.size() + response.data_.key_.size() + 64);
  resp_codec_.encode(response, buffer);
  LOG_TRACE << "conn = " << cn->id();
  cn->sendBuffer(&buffer);
  {
    LockGuard guard(lock_);
    size_t& unack = front_unacks_[cn->id()];
    unack -- ;
    if (unack == 0) cn->enableReading();
  }
}

}
}