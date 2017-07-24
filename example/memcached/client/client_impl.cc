namespace memcached
{
namespace client
{
using namespace libnet;
using namespace std;

// a tcp connection 
class ClientImpl : public NoCopyable
{
public:
  typedef std::shared_ptr<Connection> ConnectionPtr;

private:
  Client client_;
  CountDownLatch& latch_;
  ConnectionPtr connection_;

public:
  ClientImpl(EventLoop* loop, const char* host, int port, CountDownLatch& latch)
    : client_(loop, host, port),
      latch_(latch),
      connection_()
  {

  }

  ClientImpl(EventLoop* loop, const InetAddress& local_addr, CountDownLatch& latch)
  : client_(loop, local_addr),
    latch_(latch),
    connection_()
  {

  }

  void connect()
  {
    client_.setConnectionCallBack(std::bind(&ClientImpl::onConnection, this, std::placeholders::_1));
    client_.setMessageCallBack(std::bind(&ClientImpl::onMessage, this, std::placeholders::_1));
    client_.connect();
  }

  void disconnect()
  { 
    connection_.reset(); 
    client_.disconnect(); 
  }

  bool connected()
  {
    return (connection_ && connection_->connected());
  }

  void onConnection(const ConnectionPtr& connection)
  {
    if (connection->connected())
    {
      std::shared_ptr<Cache> cache = std::make_shared<Cache>();
      connection->setContext(cache);
      connection_ = connection;
      latch_.countDown();
    }
    else if (connection->disconnected())//Connection.disconnected() > ~Connection()> ~Cache()
    {
      //cache_.clear();
      //connection_.reset(); bad
    }
  }

  void send(const std::shared_ptr<Command>& message)
  { 
    std::shared_ptr<Cache> cache = std::static_pointer_cast<Cache>(connection_->getContext());
    cache->push(message);
    notifyWrite();
  }

  void onMessage(const ConnectionPtr& connection)
  { 
    connection_->loop()->assertInLoopThread();
    std::shared_ptr<Cache> cache = std::static_pointer_cast<Cache>(connection_->getContext());
    Buffer& input = connection->input();
    if (log::Logger::getLogLevel() <= libnet::log::TRACE)
      LOG_TRACE << "read = " << input.toString() ;
    while(cache->readResponse(input))// more response
    {

    }
  }

  void writeRequest()
  {
    connection_->loop()->assertInLoopThread();
    std::shared_ptr<Cache> cache = std::static_pointer_cast<Cache>(connection_->getContext());
    cache->writeRequest(connection_);
  }

  void notifyWrite()
  {
    connection_->loop()->runInLoop(std::bind(&ClientImpl::writeRequest, this));
  }
};

}
}
