#include <libnet/eventloop.h>
#include <libnet/client.h>
#include <libnet/logger.h>

using namespace libnet;

EventLoop* gEventLoop;

void cancel()
{
  gEventLoop->shutdown();
};

int main()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  EventLoop loop;
  gEventLoop = &loop;
  Client client(gEventLoop, "127.0.0.1", 9999, 1);
  client.connect();
  gEventLoop->runAfter(15000, std::bind(cancel));
  gEventLoop->loop();
}