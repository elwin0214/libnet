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
  typedef std::shared_ptr<Connection> Conn;

  HttpServer(EventLoop* loop, const char* ip, uint16_t port, EventLoopGroup* loop_group = nullptr);

  void start();
  
  void onMessage(const Conn& connPtr);

  void onConnection(const Conn& connPtr);
  
  HttpProcessor& processor(){ return processor_; };

private:
  Server server_;
  HttpProcessor processor_;
};

}
}

#endif
