#include <libnet/connection.h>
#include <libnet/server.h>
#include <libnet/client.h>
#include <libnet/eventloop.h>
#include <libnet/countdown_latch.h>
#include <libnet/logger.h>
#include <memory>
#include <functional>
#include <exception>
#include <signal.h>

using namespace std;
using namespace std::placeholders;
using namespace libnet;

typedef shared_ptr<Connection> Conn;
typedef weak_ptr<Connection> WeakConn;

//a TCP connection between proxy and backend server
class Session : public NoCopyable
{
public:
  Session(EventLoop* loop, const char* host, uint16_t port)
    : client_(loop, host, port),
      loop_(loop)
  {
    client_.setConnectionCallBack(bind(&Session::onConnect, this, _1));
    client_.setMessageCallBack(bind(&Session::onMessage, this, _1));
  }

  void setConnectedCallBack(function<void(const Conn&)> callback)
  {
    connectedCallBack = callback;
  }

  void setClosedCallBack(function<void(const Conn&)> callback)
  {
    closedCallBack = callback;
  }

  void setMessageCallBack(function<void(const Conn&)> callback)
  {
    messageCallBack = callback;
  }

  void connect()
  {
    client_.connect();
  }

  void onConnect(const Conn& conn)
  { 
    if (conn->connected()) conn_ = conn;
    if (conn->connected() && connectedCallBack) connectedCallBack(conn);
    if (conn->disconnected() && closedCallBack) closedCallBack(conn);   
  }

  void onMessage(const Conn& conn)
  {
    if (messageCallBack) messageCallBack(conn);
  }

  void send(const CString& cstring)
  {
    if (conn_)
      conn_->send(cstring);
  }

  void shutdown()
  {
    client_.disconnect();
  }

private:
  Client client_;
  EventLoop* loop_;
  Conn conn_;
  function<void(const Conn&)> connectedCallBack;
  function<void(const Conn&)> closedCallBack;
  function<void(const Conn&)> messageCallBack;
};

// a single thread TCP proxy server
class Proxy
{

public:
  Proxy(EventLoop* loop, 
        const char* local_host, 
        uint16_t local_port,
        const char* remote_host,
        uint16_t remote_port,
        CountDownLatch& latch)
    : loop_(loop),
      server_(loop, local_host, local_port),
      remote_host_(remote_host),
      remote_port_(remote_port),
      sessions_(),
      latch_(latch)
  {
    server_.setConnectionCallBack(bind(&Proxy::onConnect, this, _1));
  }

  void start()
  {
    server_.start();
  }

  void close()
  {
    for (auto itr = sessions_.begin(); itr != sessions_.end(); itr++)
    {
      auto& session = itr->second;
      if (session) session->shutdown(); // 管理session的shared_ptr在关闭前，不能erase，
    }
  }


  void onConnect(const Conn& conn)
  {
    if (conn->connected())
    { //a connection between client and proxy is established
      conn->setTcpNoDelay(true);
      shared_ptr<Session> session = make_shared<Session>(loop_, 
                                                         remote_host_, 
                                                         remote_port_ 
                                                         );
      session->connect();
      sessions_[conn->id()] = session;
      WeakConn wk_conn = conn;
      Proxy* self = this;
      CountDownLatch& latch = self->latch_;
      // server close the connection
      session->setClosedCallBack([wk_conn, self, &latch](const Conn& conn)mutable{        
        Conn front_conn = wk_conn.lock();
        if (!front_conn) return;
        front_conn->shutdown();
        int id = front_conn->id();
        conn->loop()->queueInLoop(bind(&Proxy::removeSession, self, id));// queueInLoop
        latch.countDown();
      });

      session->setConnectedCallBack([&latch](const Conn& conn)mutable{
        latch.add();
        conn->setTcpNoDelay(true);
      });
      // forward data from server to client
      session->setMessageCallBack([wk_conn](const Conn& conn){
        Conn front_conn = wk_conn.lock();
        if (!front_conn) return;
        Buffer& buffer = conn->input();
        front_conn->send(CString(buffer.beginRead(), buffer.readable()));
        buffer.clear();
      });
      // forward data from client to server
      weak_ptr<Session> wk_session = session;
      conn->setReadCallBack([wk_session](const Conn& conn){  
        shared_ptr<Session> session = wk_session.lock();
        if (!session) return;
        Buffer& buffer = conn->input();
        session->send(CString(buffer.beginRead(), buffer.readable()));
        buffer.clear();
      });
    }
    else  // client close the connection
    {
      int id = conn->id();
      auto itr = sessions_.find(id);
      if (itr == sessions_.end()) return;
      itr->second->shutdown();
    }
  }

  void removeSession(int id)
  {
    sessions_.erase(id);
  }

private:
  EventLoop* loop_;
  Server server_;
  const char* remote_host_;
  const uint16_t remote_port_;
  map<int, shared_ptr<Session>> sessions_;
  CountDownLatch& latch_;
};

function<void()> gStopCallback;
void stop(int sig)
{
  if (gStopCallback)
    gStopCallback();
};

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    LOG_ERROR << "<program> <local_ip> <local_port> <remote_ip> <remote_port> <loglevel>" ;
    exit(1);
  }
  char *local_host = static_cast<char *>(argv[1]);
  int local_port = atoi(argv[2]);
  char *remote_host = static_cast<char *>(argv[3]);
  int remote_port = atoi(argv[4]);

  int level = atoi(argv[5]);
  log::LogLevel logLevel = log::LogLevel(level);
  log::Logger::setLogLevel(logLevel);

  ::signal(SIGINT, stop);
  EventLoop loop;
  CountDownLatch closed_latch(0);
  Proxy proxy(&loop, local_host, local_port, remote_host, remote_port, closed_latch);
  gStopCallback = [&loop, &proxy, &closed_latch]()mutable{
    proxy.close();
    loop.runInterval(100, 1000, [&loop, &closed_latch]()mutable{
      if (closed_latch.count() == 0)//all session closed
        loop.shutdown();
    });
  };
  proxy.start();
  loop.loop();
  return 0;
}



