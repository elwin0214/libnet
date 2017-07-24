#ifndef __LIBNET_CLIENT_H__
#define __LIBNET_CLIENT_H__

#include "eventloop.h"
#include "connector.h"
#include "inet_address.h"
#include "mutexlock.h"
#include <functional>
#include <map>
#include <atomic>

namespace libnet
{

class Connection;
class Channel;

class Client : public NoCopyable 
{

public:
  typedef std::shared_ptr<Channel> Chan;
  typedef std::shared_ptr<Connection> Conn;
  typedef std::function<void(const Conn&)> ConnectionCallBack;
 
  Client(EventLoop* loop, const char* host, int port);

  Client(EventLoop* loop, const InetAddress& address);

  ~Client();

  bool retry(){ return retry_; }

  void enableRetry(){ retry_ = true; }

  void disableRetry() { retry_ = false; }

  void connect();

  void disconnect();

  void setConnectionCallBack(ConnectionCallBack callback) { connection_callback_ = callback; }

  void setMessageCallBack(ConnectionCallBack callback) { message_callback_ = callback; }

private:
  void newConnection(int fd);
  void disconnectInLoop();
  void removeConnection(const Conn& connPtr);

private:
  ConnectionCallBack connection_callback_;
  ConnectionCallBack message_callback_;
  //ConnectionCallBack close_connection_callback_;
  
  EventLoop* loop_;
  InetAddress server_addr_;
  std::shared_ptr<Connector> connector_;
  int next_id_;
  Conn connection_;
  std::atomic<bool> stop_;
  std::atomic<bool> retry_;
  MutexLock lock_;
 };

}
#endif