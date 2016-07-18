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
 
  Client(EventLoop* loop, const char* host, int port, int connNum);

  ~Client();

  void connect();

  void disconnect();

  void setConnectionCallBack(ConnectionCallBack callback) { connectionCallBack_ = callback; }

  void setMessageCallBack(ConnectionCallBack callback) { messageCallBack_ = callback; }

  void newConnection(int fd);

  void removeConnection(const ConnectionPtr& connPtr);

private:
  void disconnectInLoop();
  void removeConnectionInLoop(const ConnectionPtr& connPtr);

private:
  ConnectionCallBack connectionCallBack_;
  ConnectionCallBack messageCallBack_;
  ConnectionCallBack closeConnectionCallBack_;
  
  EventLoop* loop_;
  InetAddress serverAddr_;
  std::shared_ptr<Connector> connector_;
  int connId_;
  int connNum_;
  std::map<int, ConnectionPtr> connections_;
 };

}
#endif