#ifndef __LIBNET_MC_SERVER_H__
#define __LIBNET_MC_SERVER_H__
#include <libnet/nocopyable.h>
#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/mutexlock.h>
#include <memory>
#include "message.h"
#include "request_codec.h"
#include "response_codec.h"

namespace mc
{
namespace server
{
using namespace std::placeholders;
using namespace libnet;
using namespace mc::msg;

class MemServer : public libnet::NoCopyable
{
public:
  typedef std::shared_ptr<Connection> Conn;

  MemServer(EventLoop* loop, const char* ip, int port, size_t max_connections);

  void start();
  
  void onMessage(const Conn& conn);

  void onConnection(const Conn& conn);

  size_t getNumConnections() { return num_connections_; }

  void set_handler(function<void(Message&, Message&)> func) { handler_ = func; }


private:
  Server server_;
  const size_t kMaxConnecitons_;
  size_t num_connections_;
  std::function<void(Message&, Message&)> handler_;

  RequestCodec request_codec_;
  ResponseCodec response_codec_;

  // std::function<void(Message&, Buffer&)> encodec_;
  // std::function<bool(Message&, Buffer&)> decodec_;

};

}
}

#endif