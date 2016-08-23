#include <libnet/logger.h>
#include <assert.h>

using namespace libnet;
using namespace libnet::log;

typedef void (*OutputFunc)(const char* msg, int len);
typedef void (*FlushFunc)(); 

void output_func(const char* msg, int len)
{

}
void flush_func()
{

}

struct TestLogger
{

  void test_output()
  {
    Logger::setOutputFunc(output_func);
    Logger::setFlushFunc(flush_func);
    LOG_INFO << "abc" ;
  }

};
 
int main()
{
  TestLogger tl;
  tl.test_output();
  return 0;
}