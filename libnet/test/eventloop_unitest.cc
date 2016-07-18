#include <iostream>
#include <unistd.h>
#include <libnet/eventloop.h>
using namespace libnet;
 
void f()
{
    
}
int main()
{
    EventLoop loop;
    loop.runInLoop();
    loop.loop();
    loop->stop();
     
    return 0;
}