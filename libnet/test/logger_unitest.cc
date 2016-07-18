#include <cpptest.h>
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


struct TestLogger : public Test::Suite
{

  void test_output()
  {
    Logger::setOutputFunc(output_func);
    Logger::setFlushFunc(flush_func);
    LOG_INFO << "abc" ;
  }


  TestLogger()
  {
    TEST_ADD(TestLogger::test_output);
  }

};
 
int main()
{
  TestLogger tl;
  Test::TextOutput output(Test::TextOutput::Verbose);
  (tl.run(output, false));
  return 0;
}