#ifndef __LIBNET_CONNECTION_H__
#define __LIBNET_CONNECTION_H__
#include "buffer.h"
#include "cstring.h"
#include <functional>
#include <memory>

namespace libnet
{

class Socket;
class Channel;
class EventLoop;
class InetAddress;
 
class Connection : public NoCopyable, public std::enable_shared_from_this<Connection>
{

public:
  typedef std::shared_ptr<Connection> ConnectionPtr;
  typedef std::function<void(const ConnectionPtr&)>  ConnectionCallBack;

public:
  Connection(EventLoop* loop, int fd, /*InetAddress& addr, */int id);

  int id(){ return id_; }

  void set_name(const std::string& name) { name_ = name;}
  void set_name(std::string&& name) { name_ = std::move(name);}

  const std::string& get_name(){ return name_;}

  EventLoop* loop(){ return loop_; }

  //void sendString(const CString& cstring);
  void sendBuffer(Buffer* buffer)
  {
    send(CString(buffer->beginRead(), buffer->readable()));
  }

  
  //void sendWithLen(const char* str, size_t len) { sendString(CString(str, len)); }
  //void send(const char* str) { sendString(CString(str));}
  void send(const CString& cstring);

  Buffer& input() {return input_;}
  Buffer& output() {return output_;}

  bool connected() {return state_ == kConnected;}
  bool disconnected() {return state_ == kDisConnected;}

  void establish();
  void destroy();

  void shutdown();
  void shutdownInLoop();

  void setConnectionCallBack(ConnectionCallBack callback) {connection_callback_ = callback; }//打开、关闭的callback
  void setReadCallBack(ConnectionCallBack callback) {read_callback_ = callback; }//读到sock的数据需要传递给外部
  void setCloseCallBack(ConnectionCallBack callback) {close_callback_ = callback; }//关闭connection需要通知到外部


  void handleRead();
  void handleWrite();
  void handleClose();
  void handleError();

  void setContext(const std::shared_ptr<void>& context) { context_ = context; }
  std::shared_ptr<void>& getContext() { return context_; }

  void setWeakContext(const std::weak_ptr<void>& wk_context) { wk_context_ = wk_context; }
  std::weak_ptr<void>& getWeakContext() { return wk_context_; }

  const char* stateToString();
  ~Connection();
private:
  void sendInLoop(const CString& cstring);
  
private:
  enum State {kConnecting, kConnected, kDisConnecting, kDisConnected};

  EventLoop* loop_;
  State state_;
  ConnectionCallBack connection_callback_;
  ConnectionCallBack read_callback_;
  ConnectionCallBack close_callback_;
  std::shared_ptr<Channel> channel_;
  std::shared_ptr<Socket> socket_;
  Buffer input_;
  Buffer output_;
  int id_;
  std::string name_;
  std::shared_ptr<void> context_;
  std::weak_ptr<void> wk_context_;
};

}
#endif