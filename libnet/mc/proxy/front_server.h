#ifndef __LIBNET_MEMCACHED_PROXY_FRONT_H__
#define __LIBNET_MEMCACHED_PROXY_FRONT_H__

#include <map>
#include <libnet/nocopyable.h>
#include <libnet/server.h>
#include <libnet/mc/message.h>
#include <libnet/mc/request_codec.h>
#include <libnet/mc/response_codec.h>

namespace mc
{
namespace client 
{ 
class Command; 
}
}

namespace mc
{
namespace proxy
{
using namespace libnet;
using namespace std;
using namespace mc::client;
using namespace mc::msg;

class FrontServer : public NoCopyable
{
public:
  typedef shared_ptr<Connection> Conn;
  typedef shared_ptr<Command> Cmd;
  typedef shared_ptr<Message> Msg;

public:
  FrontServer(EventLoop* loop, 
              EventLoopGroup* loop_group,
              const InetAddress& local_address);

  void start();
  void onConnection(const Conn& cn);
  void onMessage(const Conn& cn);
  void send(const Conn& cn, Message& message);

  void setMessageCallBack(function<void(const Conn&, Message&)> callback) 
  {
    message_callback_ = callback;
  }

private:
  Server server_;
  map<int, Conn> front_conns_;
  function<void(const Conn&, Message&)> message_callback_;
  RequestCodec req_codec_;
  ResponseCodec resp_codec_;
};

}
}

#endif