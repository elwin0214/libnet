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

HttpServer::HttpServer(EventLoop* loop, const char* ip, int port, int threads)
  : server_(loop, ip, port, threads)
{

};

void HttpServer::start()
{
  server_.setConnectionCallBack(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallBack(std::bind(&HttpServer::onMessage, this, std::placeholders::_1));
  server_.start();
};

void HttpServer::onConnection(const ConnectionPtr& connection)
{
  LOG_DEBUG << "onConnection connected=" << (connection->connected()) ;
  if (connection->connected())
  {
    std::shared_ptr<HttpContext> context = std::make_shared<HttpContext>();
    connection->setContext(context);
    context->getResponse().setSendCallback(std::bind(&Connection::sendBuffer, connection.get(), std::placeholders::_1));
    context->getResponse().setSendStringCallback(std::bind(&Connection::sendString, connection.get(), std::placeholders::_1));

  }  
};

void HttpServer::onMessage(const ConnectionPtr& connection)
{
  std::shared_ptr<HttpContext> context = std::static_pointer_cast<HttpContext>(connection->getContext());

  Buffer &input = connection->input();
  bool r = httpProcessor_.process(input, *context);
  LOG_DEBUG << "onConnection " <<  (context->getState()) << " process = " << r ;
  if (!r)
  {
    connection->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    connection->shutdown();
    return;
  }
  HttpResponse& response = context->getResponse();

  if (context->getState() == HttpContext::kBodySent || context->getState() == HttpContext::kAllChunkSent)
  {
    if (response.isClose())
      connection->shutdown();
    context->reset();
  }

};


}
}
