#include "echo_server.h"

using namespace libnet;

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        LOG_ERROR << "<program> <ip> <port> <threads> <loglevel>" ;
        exit(1);
    }
    char *host = static_cast<char *>(argv[1]);
    int port = atoi(argv[2]); 
    int threads = atoi(argv[3]);
    int level = atoi(argv[4]);
    log::LogLevel logLevel = log::LogLevel(level);
    setLogLevel(logLevel);
    EventLoop loop;
    EchoServer server(&loop, host, port, threads);
    server.start();
    loop.loop();
    return 0;
}