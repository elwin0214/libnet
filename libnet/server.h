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
  Server(EventLoop* loop, const char* host, int port, int workersNum);
  ~Server();

  void start();
  void setConnectionCallBack(ConnectionCallBack callback) { connectionCallBack_ = callback; }
  void setMessageCallBack(ConnectionCallBack callback) { messageCallBack_ = callback; }

  void newConnection(int fd, InetAddress &addr);
  void removeConnection(const ConnectionPtr &connPtr);
  void removeConnectionInLoop(const ConnectionPtr &connPtr);

  
private:
  EventLoop* loop_;
  InetAddress localAddr_;
  Acceptor acceptor_;
  EventLoopGroupPtr loopGroupPtr_;
  
  int nextConId_; 
  MutexLock lock_;
  bool started_;
  
  std::map<int, ConnectionPtr> connections_;

  ConnectionCallBack connectionCallBack_;
  ConnectionCallBack messageCallBack_;
};

}

#endif