#include <gperftools/profiler.h>
#include <libnet/thread.h>

using namespace std;
using namespace libnet;

void func0()
{
    int i = 0;
    while (i < 100000) {
        ++i;
    }  
}


void func1() {
    int i = 0;
    while (i < 100000) {
        ++i;
    }  
}
void func2() {
    int i = 0;
    while (i < 200000) {
        ++i;
    }  
}
void func3() {
    for (int i = 0; i < 1000; ++i) {
        func1();
        func2();
    }  
}
int main(){
    ProfilerStart("test.prof");
    func3();
    Thread thread(std::bind(func0));
    thread.start();
    thread.join();
    ProfilerStop();
    return 0;
}
