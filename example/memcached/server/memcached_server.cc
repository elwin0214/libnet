#include <memory>
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/buffer.h>
#include <libnet/logger.h>
#include "item.h"
#include "memcached_server.h"
#include "memcached_context.h"

namespace memcached
{
namespace server
{
using namespace libnet;

MemcachedServer::MemcachedServer(EventLoop* loop, const char* ip, int port, size_t max_connections)
  : server_(loop, ip, port), //single thread server
    kMaxConnecitons_(max_connections),
    num_connections_(0),
    requestCodec_(),
    responseCodec_()
{
};

void MemcachedServer::start()
{
  server_.setConnectionCallBack(std::bind(&MemcachedServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&MemcachedServer::onMessage, this, std::placeholders::_1));
  server_.start();
};

void MemcachedServer::onConnection(const Conn& conn)
{
  if (conn->connected())
  {
    num_connections_++;
    if (num_connections_ > kMaxConnecitons_)
    {
      conn->shutdown();
      return;
    }
    std::shared_ptr<MemcachedContext> ctx = std::make_shared<MemcachedContext>();
    conn->setContext(ctx);
  }
  else
  {
    num_connections_-- ;
  }
};

void MemcachedServer::onMessage(const Conn& conn)
{
  std::shared_ptr<MemcachedContext> context = std::static_pointer_cast<MemcachedContext>(conn->getContext());
  Message& request = context->request();
  Buffer& in = conn->input();
  if (!requestCodec_.decode(request, in)) return;
  if (request.stat_.code_ != kSucc)
  {
    conn->send("ERROR\r\n");
    request.reset();
    return;
  }
  LOG_TRACE << " op = " << request.op()  << " key = "  << request.data_.key_;
  Message response(request.data_.op_);
  handler_(request, response);
  request.reset();
  Buffer buffer(0, response.data_.value_.size() + response.data_.key_.size() + 64);
  responseCodec_.encode(response, buffer);
  conn->sendBuffer(&buffer);
};

}
}