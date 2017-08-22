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

  /*struct Session
  {
    Session(size_t unack, Connection* owner)
      : unack_(unack),
        owner_(owner)
    {
    };
    Session(const Session& s)
      : unack_(s.unack_),
        owner_(s.owner_)
    {
    };
    Connection* owner_;
    size_t unack_;
  }; */

public:
  FrontServer(EventLoop* loop, 
              EventLoopGroup* loop_group,
              const InetAddress& local_address,
              size_t unack = 100);

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
  std::map<int, size_t> front_unacks_;
  MutexLock lock_;
  function<void(const Conn&, Message&)> message_callback_;
  RequestCodec req_codec_;
  ResponseCodec resp_codec_;
  size_t unack_;
};

}
}

#endif