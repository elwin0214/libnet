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

    //int fd();
    int id(){ return id_; }
    EventLoop* loop(){ return loop_; }
    //void start();

    //void send(const char *str, int len);

    void sendString(const CString& cstring);
    void send(const std::shared_ptr<Buffer>& buffer);
    void sendBuffer(Buffer* buffer);

    void sendStringInLoop(const CString& cstring);
    void sendInLoop(const std::shared_ptr<Buffer>& buffer);
    void sendBufferInLoop(Buffer* buffer);

    void send(const char* str) { sendString(CString(str));}

    Buffer& input() {return inputBuffer_;}
    Buffer& output() {return outputBuffer_;}

    bool connected() {return state_ == CONNECTED;}
    bool disconnected() {return state_ == DIS_CONNECTED;}

    void establish();
    void destroy();

    void shutdown();
    void shutdownInLoop();

    void setConnectionCallBack(ConnectionCallBack callback) {connectionCallBack_ = callback; }//打开、关闭的callback
    void setReadCallBack(ConnectionCallBack callback) {readCallBack_ = callback; }//读到sock的数据需要传递给外部
    void setCloseCallBack(ConnectionCallBack callback) {closeCallBack_ = callback; }//关闭connection需要通知到外部


    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    void setContext(void* context) { context_ = context; }
    void* getContext() { return context_; }
    const char* stateToString();
    ~Connection();

private:
    enum State {CONNECTING, CONNECTED, DIS_CONNECTING, DIS_CONNECTED};

    EventLoop* loop_;
    State state_;
    ConnectionCallBack connectionCallBack_;
    ConnectionCallBack readCallBack_;
    ConnectionCallBack closeCallBack_;
    std::shared_ptr<Channel> channel_;
    std::shared_ptr<Socket> socket_;
    //EventLoop *
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    int id_;

    void* context_;
};

}
#endif