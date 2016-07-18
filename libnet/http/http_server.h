#ifndef __LIBNET_HTTP_HTTPSERVER_H__
#define __LIBNET_HTTP_HTTPSERVER_H__

#include <memory>
#include <libnet/connection.h>  
#include <libnet/server.h>  
#include <libnet/http/http_processor.h> 


namespace libnet
{
namespace http
{
class HttpRequest;
class HttpContext;

class HttpServer
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

  HttpServer(EventLoop* loop, const char* ip, int port, int threads);

  void start();
  
  void onMessage(const ConnectionPtr& connPtr);

  void onConnection(const ConnectionPtr& connPtr);
  
  HttpProcessor& processor(){ return httpProcessor_; };

private:
  Server server_;
  HttpProcessor httpProcessor_;
};

}
}

#endif
