#ifndef __LIBNET_HTTP_HTTPPROCESSOR_H__
#define __LIBNET_HTTP_HTTPPROCESSOR_H__

#include <libnet/http/http_request.h>
#include <libnet/http/http_context.h>
#include <libnet/connection.h>
#include <libnet/buffer.h>
#include <libnet/logger.h>

namespace libnet
{
namespace http
{

struct DefaultHandler
{
  size_t handleBody(const Buffer& buffer, size_t allowed, HttpContext& context)
  {
    return allowed;
  }

  void handleRequest(const Buffer& buffer, HttpContext& context)
  {
    HttpResponse& response = context.getResponse();
    response.setStatus(HttpResponse::NotFound);
    response.setClose(true);
    response.flush();
  }
};

class HttpProcessor
{
public:
  typedef std::function<int(const Buffer&, size_t, HttpContext&)> HttpReaderCallBack;
  typedef std::function<void(const Buffer&, HttpContext&)> HttpCallBack;

public:
  HttpProcessor();

  bool process(Buffer &input, HttpContext &context);
  void setBodyReaderCallBack(HttpReaderCallBack callback) { bodyReaderCallBack_ = callback; };
  void setHeaderHandlerCallBack(HttpCallBack callback) { headerHandlerCallBack_ = callback; };//
  void setRequestHandlerCallBack(HttpCallBack callback) { requestHandlerCallBack_ = callback; };

private:
  bool parseRequest(Buffer &input, HttpContext &context);
  bool parseRequestLine(const char* begin, const char* end, HttpContext& context);
  bool parseRequestHeader(const char* begin, const char* end, HttpContext &context);
  bool processHeaders(HttpContext &context);
  // size - the consumed size
  size_t processBody(Buffer &input, size_t size, HttpContext &context);

private:
  DefaultHandler defaultHandler_;
  HttpCallBack headerHandlerCallBack_; //解析完头部后调用
  HttpReaderCallBack bodyReaderCallBack_;// 读取body
  HttpCallBack requestHandlerCallBack_; //读完后调用
};

}
}

#endif
