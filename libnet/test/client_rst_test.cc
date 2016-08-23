#include <libnet/eventloop.h>
#include <libnet/client.h>
#include <libnet/logger.h>

using namespace libnet;

EventLoop* gLoop;

void cancel()
{
  gLoop->shutdown();
};

int main()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  EventLoop loop;
  gLoop = &loop;
  Client client(gLoop, "127.0.0.1", 9999);
  client.connect();
  gLoop->runAfter(15000, std::bind(cancel));
  gLoop->loop();
}