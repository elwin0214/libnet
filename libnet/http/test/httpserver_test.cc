#include <functional>
#include <iostream>
#include <libnet/logger.h>
#include <libnet/http/http_context.h>
#include <libnet/http/http_server.h>

using namespace libnet;
using namespace libnet::http;
using namespace std::placeholders;

struct OkHandler
{
  size_t handleBody(const Buffer& buffer, size_t allowed, HttpContext& context)
  {
    return allowed;
  }

  void handleRequest(const Buffer& buffer, HttpContext& context)
  {
    HttpRequest& request = context.getRequest();
    HttpResponse& response = context.getResponse();
    if (request.getPath() != "/chunked")
    {
      response.setStatus(HttpResponse::OK);
      response.send("123", 3);
      response.flush();
    }
    else
    {
      response.setChunked(true);
      response.setStatus(HttpResponse::OK);
      response.send("123", 3);
      response.flush();
      response.send("45678", 5);
      response.flush();
      response.finish();
    }

  }
};

int main(int argc, char *argv[])
{
  if (argc < 5)
  {
      LOG_ERROR << "<program> <ip> <port> <threads> <loglevel>" ;
      exit(1);
  }
  char *host = static_cast<char *>(argv[1]);
  int port = atoi(argv[2]); 
  int threads = atoi(argv[3]);
  int level = atoi(argv[4]);
  log::LogLevel logLevel = log::LogLevel(level);
  setLogLevel(logLevel);
  EventLoop loop;
  HttpServer server(&loop, host, port);
  OkHandler handler;
  server.processor().setRequestHandlerCallBack(std::bind(&OkHandler::handleRequest, &handler, _1, _2));
  server.start();
  loop.loop();
  return 0;
}
