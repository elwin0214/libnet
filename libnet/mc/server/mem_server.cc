#include <memory>
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/buffer.h>
#include <libnet/logger.h>
#include <libnet/mc/item.h>
#include <libnet/mc/mem_server.h>
#include <libnet/mc/context.h>

namespace mc
{
namespace server
{
using namespace libnet;

MemServer::MemServer(EventLoop* loop, const char* ip, int port, EventLoopGroup* loop_group, size_t max_connections)
  : server_(loop, ip, port, loop_group), 
    kMaxConnecitons_(max_connections),
    num_connections_(0),
    request_codec_(),
    response_codec_()
{
};

void MemServer::start()
{
  server_.setConnectionCallBack(std::bind(&MemServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&MemServer::onMessage, this, std::placeholders::_1));
  server_.start();
};

void MemServer::onConnection(const Conn& conn)
{

  if (conn->connected())
  {
    num_connections_++;
    if (num_connections_ > kMaxConnecitons_)
    {
      conn->shutdown();
      return;
    }
    std::shared_ptr<Context> ctx = std::make_shared<Context>();
    conn->setContext(ctx);
  }
  else
  {
    num_connections_-- ;
  }
};

void MemServer::onMessage(const Conn& conn)
{
  std::shared_ptr<Context> context = std::static_pointer_cast<Context>(conn->getContext());
  Message& request = context->request();
  Buffer& in = conn->input();
  while (true)
  {
    if (!request_codec_.decode(request, in)) return;
    if (request.stat_.code_ != kSucc)
    {
      conn->send("ERROR\r\n");
      request.reset();
      continue;
    }
    if (request.data_.op_ == kQuit)
    {
      LOG_DEBUG << " conn = " << conn->get_name() << " quit";
      conn->shutdown();
      return;
    }
    LOG_TRACE << " op = " << request.op()  << " key = "  << request.data_.key_;
    Message response(request.data_.op_);
    handler_(request, response);
    request.reset();
    Buffer buffer(0, response.data_.value_.size() + response.data_.key_.size() + 64);
    response_codec_.encode(response, buffer);
    conn->sendBuffer(&buffer);
  }

};

}
}