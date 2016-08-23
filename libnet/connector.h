#ifndef __LIBNET_CONNECTOR_H__
#define __LIBNET_CONNECTOR_H__

#include <memory>
#include <map>
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
  //typedef std::map<int, ChannelPtr> Channels;
  typedef std::function<void(int)> NewConnectionCallBack;


  Connector(EventLoop* loop, const InetAddress& serverAddress);

  ~Connector();
  
  void start();

  void stop();

  void setNewConnectionCallBack(NewConnectionCallBack callback) { newConnectionCallBack_ = callback; };

private:
  void retry();

  void connect();

  void connectInLoop();

  void registerConnect(int fd);

  void handleWrite(int fd);

  void handleError(int fd);

  void removeChannelInLoop(int fd, bool close);

private:
  enum State
  {
    kConnecting,
    kConnected,
    kDisConnected
  };

  bool stop_;
  State state_;
  EventLoop* loop_;
  InetAddress serverAddress_;
  //Channels channels_; // 
  ChannelPtr channel_;
  NewConnectionCallBack newConnectionCallBack_;
  MutexLock lock_;

};

}

#endif