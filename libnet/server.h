#ifndef __LIBNET_SERVER_H__
#define __LIBNET_SERVER_H__

#include <libnet/inet_address.h>
#include <libnet/acceptor.h>
#include <libnet/eventloop.h>
#include <libnet/mutexlock.h>
#include <libnet/condition.h>
#include <vector>
#include <map>
#include <memory>


namespace libnet
{

class Connection;
class EventLoop;
class EventLoopGroup;
class Acceptor;

class Server : public NoCopyable 
{
public: 
  typedef std::shared_ptr<Connection> Conn;
  typedef std::function<void(const Conn&)> ConnectionCallBack;
  typedef EventLoop* Loop;
  //typedef std::shared_ptr<EventLoopGroup> LoopGroup;

public:
  Server(EventLoop* loop, const char* host, uint16_t port, EventLoopGroup* loop_group = nullptr);
  Server(EventLoop* loop, const InetAddress& address, EventLoopGroup* loop_group = nullptr);

  ~Server();

  void start();
  void setConnectionCallBack(ConnectionCallBack callback) { connection_callback_ = callback; }
  void setMessageCallBack(ConnectionCallBack callback) { message_callback_ = callback; }

  void newConnection(int fd, InetAddress &addr);
  void removeConnection(const Conn& conn);
  void removeConnectionInLoop(const Conn& conn);

  
private:
  EventLoop* loop_;
  InetAddress local_addr_;
  Acceptor acceptor_;
  EventLoopGroup* loop_group_;
  //EventLoopGroupPtr loop_group_;
  
  int next_id_; 
  MutexLock lock_;
  bool started_;
  
  std::map<int, Conn> conns_;

  ConnectionCallBack connection_callback_;
  ConnectionCallBack message_callback_;
};

}

#endif