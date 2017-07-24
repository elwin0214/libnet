#include <functional>
#include <algorithm> 
#include <libnet/logger.h>
#include <libnet/buffer.h>
#include <libnet/http/http_server.h>
#include <libnet/http/http_request.h>
#include <libnet/http/http_response.h>
#include <libnet/http/http_context.h>

namespace libnet
{
namespace http
{

HttpServer::HttpServer(EventLoop* loop, const char* ip, uint16_t port, EventLoopGroup* loop_group)
  : server_(loop, ip, port, loop_group)
{

};

void HttpServer::start()
{
  server_.setConnectionCallBack(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&HttpServer::onMessage, this, std::placeholders::_1));
  server_.start();
};

void HttpServer::onConnection(const Conn& conn)
{
  LOG_DEBUG << "onConnection connected=" << (conn->connected()) ;
  if (conn->connected())
  {
    std::shared_ptr<HttpContext> context = std::make_shared<HttpContext>();
    conn->setContext(context);
    context->getResponse().setSendCallback(std::bind(&Connection::send, conn.get(), std::placeholders::_1));
  }  
};

void HttpServer::onMessage(const Conn& conn)
{
  std::shared_ptr<HttpContext> context = std::static_pointer_cast<HttpContext>(conn->getContext());

  Buffer &input = conn->input();
  bool r = processor_.process(input, *context);
  LOG_DEBUG << "onConnection " <<  (context->getState()) << " process = " << r ;
  if (!r)
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
    return;
  }
  HttpResponse& response = context->getResponse();

  if (context->getState() == HttpContext::kBodySent || context->getState() == HttpContext::kAllChunkSent)
  {
    if (response.isClose())
      conn->shutdown();
    context->reset();
  }

};


}
}
