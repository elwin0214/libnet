#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include "../async_client.h"

using namespace libnet;
using namespace memcached::client;

EventLoop* gLoop; 
AsyncClient* gClient;


void test_set()
{
  assert(gClient->set("a", 2000, "cc"));
  assert(gClient->set("b", 2000, "cc"));
  assert("cc" == gClient->get("b"));
}

void test_rem()
{
  gClient->remove("a");
  assert((!(gClient->remove("a"))));
  assert(gClient->set("a", 2000, "cc"));
  assert(gClient->remove("a"));
}


void test_add()
{
  gClient->remove("a");
  assert(!(gClient->remove("a")));
  assert(gClient->add("a", 2000, "1"));
  assert(!(gClient->add("a", 2000, "999")));
  assert(2 == (gClient->incr("a", 1)));
  assert(3 == (gClient->incr("a", 1)));

  uint32_t last_incr;
  for (int i = 0 ; i < 100 ; i++)
  {
    last_incr = gClient->incr("a", 1);
  }
  assert( last_incr == 103);


  for (int i = 0 ; i < 100 ; i++)
  {
    last_incr = gClient->decr("a", 1);
  }
  assert( last_incr == 3);
}



int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        LOG_ERROR << "<program> <memcached ip> <memcached port> <loglevel>" ;
        exit(1);
    }
    char *host = static_cast<char *>(argv[1]);
    int port = atoi(argv[2]); 
    int level = atoi(argv[3]);
    log::LogLevel logLevel = log::LogLevel(level);
    setLogLevel(logLevel);

    EventLoopThread loopThread("loop");
    loopThread.start();
    CountDownLatch latch(1);
    gLoop = loopThread.getLoop();
    AsyncClient client(gLoop, host, port, latch);
    gClient = &client;
    client.connect();
    latch.wait();
 
    test_set();
    test_rem();
    test_add();

    return 0;
}