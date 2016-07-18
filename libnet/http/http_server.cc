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

void HttpServer::onConnection(const ConnectionPtr& connPtr)
{
  LOG_DEBUG << "onConnection connected=" << (connPtr->connected()) ;
  if (connPtr->connected())
  {
    HttpContext* context = new HttpContext();
    connPtr->setContext(context);
    context->getResponse().setSendCallback(std::bind(&Connection::sendBuffer, connPtr, std::placeholders::_1));
    context->getResponse().setSendStringCallback(std::bind(&Connection::sendString, connPtr, std::placeholders::_1));

  }
  else
  {
    HttpContext* ctxPtr = static_cast<HttpContext*>(connPtr->getContext());
    if (NULL == ctxPtr) return;
    delete ctxPtr;
  }
  
};

void HttpServer::onMessage(const ConnectionPtr& connPtr)
{
  HttpContext *const context = static_cast<HttpContext*>(connPtr->getContext());
  Buffer &input = connPtr->input();
  bool r = httpProcessor_.process(input, *context);
  LOG_DEBUG << "onConnection " <<  (context->getState()) << " process = " << r ;
  if (!r)
  {
    connPtr->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    connPtr->shutdown();
    return;
  }
  HttpResponse& response = context->getResponse();

  if (context->getState() == HttpContext::kBodySent || context->getState() == HttpContext::kAllChunkSent)
  {
    if (response.isClose())
      connPtr->shutdown();
    context->reset();
  }

};


}
}
