#include <libnet/logger.h>
#include <libnet/eventloop.h>
#include <libnet/eventloop_thread.h>
#include <libnet/thread.h>
#include "memcached_client.h"

using namespace libnet;

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        LOG_ERROR << "<program> <ip> <port> <loglevel>" ;
        exit(1);
    }
    char *host = static_cast<char *>(argv[1]);
    int port = atoi(argv[2]); 
    int level = atoi(argv[3]);
    log::LogLevel logLevel = log::LogLevel(level);
    setLogLevel(logLevel);
    EventLoopThread loopThread("loop");
    loopThread.start();
    CountDownLatch countDownLatch(1);
    MemcachedClient client(loopThread.getLoop(), host, port, countDownLatch);
    client.connect();
    countDownLatch.wait();
    std::shared_ptr<Message> setMsg = client.set("a", 2000, "cc");
    setMsg->wait();
    LOG_INFO << "code=" <<(setMsg->code()) << " result=" << (setMsg->result()) ;

    std::shared_ptr<Message> setMsg2= client.set("aa", 2000, "aacc");
    setMsg2->wait();
    LOG_INFO << "code=" <<(setMsg2->code()) << " result=" << (setMsg2->result()) ;

    std::shared_ptr<Message> getMsg = client.get("a");
    getMsg->wait();
    LOG_INFO << "code=" <<(getMsg->code()) << " result=" << (getMsg->result()) ;

    assert("cc" == (getMsg->result()));

    loopThread.getLoop()->shutdown();

    return 0;
}