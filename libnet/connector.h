#ifndef __LIBNET_CONNECTOR_H__
#define __LIBNET_CONNECTOR_H__

#include <memory>
#include <atomic>
#include <functional>
#include "atomic.h"
#include "inet_address.h"
#include "mutexlock.h"

namespace libnet
{
class EventLoop;
class Connection;
class Channel;

class Connector : public NoCopyable, public std::enable_shared_from_this<Connector>
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::function<void(ConnectionPtr)> ConnectionCallBack;
  typedef std::unique_ptr<Channel> ChannelPtr;
  typedef std::function<void(int)> NewConnectionCallBack;


  Connector(EventLoop* loop, const InetAddress& serverAddress);

  ~Connector();
  
  void start();

  void restart();

  void stop();

  void setNewConnectionCallBack(NewConnectionCallBack callback) { new_connection_callback_ = callback; };

private:
  void retry();

  void startInLoop();

  void stopInLoop();

  void connectInLoop();

  void registerConnect(int fd);

  void handleWrite(int fd);

  void handleError(int fd);

  void removeChannelInLoop();

private:
  enum State
  {
    kConnecting,
    kConnected,
    kDisConnected
  };

  std::atomic<bool> stop_;
  State state_;
  EventLoop* loop_;
  InetAddress server_address_;
  ChannelPtr channel_;
  NewConnectionCallBack new_connection_callback_;
  MutexLock lock_;

};

}

#endif