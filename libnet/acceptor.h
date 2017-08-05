#ifndef __LIBNET_ACCEPTOR_H__
#define __LIBNET_ACCEPTOR_H__

#include <functional>
#include <memory>  //ubuntu gcc 需要include
#include <libnet/nocopyable.h>
#include <libnet/socket.h>

namespace libnet
{
class EventLoop;
class InetAddress;
class Channel;
class Socket;

class Acceptor : public NoCopyable
{
public:
  typedef std::function<void(int fd, InetAddress& addr)>  ConnectionCallback;


public:
  Acceptor(EventLoop *loop, const InetAddress& addr, int backlog);

  ~Acceptor();

  void start();

  void listen();

  void close();

  void handleClose();

  void handleRead();
 
  void setNewConnectionCallback(ConnectionCallback callback) { new_conn_callback_ = callback; }

private:
  int backlog_;
  bool closed_;
  EventLoop *loop_;
  Socket socket_;
  std::shared_ptr<Channel> channel_;
  ConnectionCallback new_conn_callback_;

};

}

#endif