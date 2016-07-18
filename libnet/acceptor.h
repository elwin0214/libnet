#ifndef __LIBNET_ACCEPTOR_H__
#define __LIBNET_ACCEPTOR_H__

#include <functional>
#include <memory>  //ubuntu gcc 需要include
#include "nocopyable.h"
#include "socket.h"

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
  Acceptor(EventLoop *loop, InetAddress& listenAddr, int backlog);

  ~Acceptor();

  void start();

  void listen();

  void close();

  void handleClose();

  void handleRead();
 
  void setNewConnectionCallback(ConnectionCallback callback) { newConnectionCallback_ = callback; }

private:
  int backlog_;
  bool closed_;
  EventLoop *loop_;
  Socket socket_;
  std::shared_ptr<Channel> channel_;
  ConnectionCallback newConnectionCallback_;

};

}

#endif