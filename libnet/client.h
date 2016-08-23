#ifndef __LIBNET_CLIENT_H__
#define __LIBNET_CLIENT_H__

#include "eventloop.h"
#include "connector.h"
#include "inet_address.h"

#include <functional>
#include <map>

namespace libnet
{

class Connection;
class Channel;

class Client : public NoCopyable 
{

public:
  typedef std::shared_ptr<Channel> ChannelPtr;
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::function<void(const ConnectionPtr&)> ConnectionCallBack;
 
  Client(EventLoop* loop, const char* host, int port);

  ~Client();

  void connect();

  void disconnect();

  void setConnectionCallBack(ConnectionCallBack callback) { connectionCallBack_ = callback; }

  void setMessageCallBack(ConnectionCallBack callback) { messageCallBack_ = callback; }

private:
  void newConnection(int fd);
  void disconnectInLoop();
  void removeConnection(const ConnectionPtr& connPtr);
  //void removeConnectionInLoop(const ConnectionPtr& connPtr);

private:
  ConnectionCallBack connectionCallBack_;
  ConnectionCallBack messageCallBack_;
  ConnectionCallBack closeConnectionCallBack_;
  
  EventLoop* loop_;
  InetAddress serverAddr_;
  std::shared_ptr<Connector> connector_;
  int connId_;
  ConnectionPtr connectionPtr_;
  //std::map<int, ConnectionPtr> connections_;
 };

}
#endif