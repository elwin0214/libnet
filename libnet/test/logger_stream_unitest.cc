#include <libnet/logger.h>
#include <assert.h>

using namespace libnet;
using namespace libnet::log;

struct TestLoggerStream 
{
  void test_append()
  {
    LoggerStream stream;
    stream << "abc" ;
    assert(stream.buffer().toString() == "abc");

    stream << "def" ;
    assert(stream.buffer().toString() == "abcdef");

    stream << 100 ;
    assert(stream.buffer().toString() == "abcdef100");

  }

  void test_append_over()
  {
     LoggerStream stream;
     for (int i = 0 ; i <= 2000 ; i++)
     {
        stream << "a" ;
     }
     stream << "abc" ;
     stream << "def" ;
     assert(stream.buffer().remain() == 0);
     assert(stream.buffer().size() == 1024);
  }

  void test_close()
  {
     LoggerStream stream;
    
     stream << "abc" ;
     stream.close();
     stream << "def" ;
     assert(stream.buffer().toString() == "abc");
  }


};
 
int main()
{
  TestLoggerStream tls;

  tls.test_append();
  tls.test_append_over();
  tls.test_close();

  return 0;
}

 