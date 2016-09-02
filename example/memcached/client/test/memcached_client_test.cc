#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include "../memcached_client.h"
#include "../future.h"

using namespace libnet;
using namespace memcached::client;

EventLoop* gLoop; 
MemcachedClient* gClient;


void test_set()
{
  std::shared_ptr<Future> set1 = gClient->set("a", 2000, "cc");
  set1->wait();
  assert(set1->code() == kSucc);
  
  std::shared_ptr<Future> set2 = gClient->set("b", 2000, "cc");
  set2->wait();
  assert(set2->code() == kSucc);

  std::shared_ptr<Future> get = gClient->get("b");
  get->wait();
  assert(get->code() == kSucc);
  assert(get->result() ==  "cc");
}

void test_rem()
{
  std::shared_ptr<Future> rem = gClient->remove("a");
  rem->wait();
  //assert(rem->code() == kSucc);
  
  std::shared_ptr<Future> rem2 = gClient->remove("a");
  rem2->wait();
  assert(rem2->code() == kFail);

  std::shared_ptr<Future> set = gClient->set("a", 2000, "cc");
  set->wait();
  assert(set->code() == kSucc);

  std::shared_ptr<Future> rem3 = gClient->remove("a");
  rem3->wait();
  assert(rem3->code() == kSucc);
}


void test_add()
{
  std::shared_ptr<Future> rem = gClient->remove("a");
  rem->wait();
  

  std::shared_ptr<Future> add = gClient->add("a", 2000, "1");
  add->wait();
  assert(add->code() == kSucc);


  std::shared_ptr<Future> add2 = gClient->add("a", 2000, "999");
  add2->wait();
  assert(add2->code() == kFail);

  std::shared_ptr<Future> incr = gClient->incr("a", 1);
  incr->wait();
  assert(incr->code() == kSucc);
  assert(incr->result() == "2");

  std::string last_incr;
  for (int i = 0 ; i < 100 ; i++)
  {
    std::shared_ptr<Future> incr = gClient->incr("a", 1);
    incr->wait();
    assert(incr->code() == kSucc);
    last_incr = incr->result();
  }

  assert( last_incr == "102");


  for (int i = 0 ; i < 100 ; i++)
  {
    std::shared_ptr<Future> decr = gClient->decr("a", 1);
    decr->wait();
    assert(decr->code() == kSucc);
    last_incr = decr->result();
  }
    
  assert( last_incr == "2");


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
    MemcachedClient client(gLoop, host, port, latch);
    gClient = &client;
    client.connect();
    latch.wait();
 
    test_set();
    test_rem();
    test_add();

    return 0;
}