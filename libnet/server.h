#ifndef __LIBNET_SERVER_H__
#define __LIBNET_SERVER_H__

#include "inet_address.h"
#include "acceptor.h"
#include "eventloop.h"
#include "mutexlock.h"
#include "condition.h"
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
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::function<void(const ConnectionPtr&)> ConnectionCallBack;
  typedef std::shared_ptr<Acceptor> AcceptorPtr;
  typedef EventLoop* EventLoopPtr;
  typedef std::shared_ptr<EventLoopGroup> EventLoopGroupPtr;

public:
  Server(EventLoop* loop, const char* host, int port, int workers);
  ~Server();

  void start();
  void setConnectionCallBack(ConnectionCallBack callback) { connection_callBack_ = callback; }
  void setMessageCallBack(ConnectionCallBack callback) { message_callBack_ = callback; }

  void newConnection(int fd, InetAddress &addr);
  void removeConnection(const ConnectionPtr &connection);
  void removeConnectionInLoop(const ConnectionPtr &connection);

  
private:
  EventLoop* loop_;
  InetAddress local_addr_;
  Acceptor acceptor_;
  EventLoopGroupPtr loop_group_;
  
  int next_id_; 
  MutexLock lock_;
  bool started_;
  
  std::map<int, ConnectionPtr> connections_;

  ConnectionCallBack connection_callBack_;
  ConnectionCallBack message_callBack_;
};

}

#endif