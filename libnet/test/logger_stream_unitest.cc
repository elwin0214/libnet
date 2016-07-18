#include <cpptest.h>
#include <libnet/logger.h>
#include <assert.h>

using namespace libnet;
using namespace libnet::log;

struct TestLoggerStream  : public Test::Suite
{
  void test_append()
  {
    LoggerStream stream;
    stream << "abc" ;
    assert(stream.buffer().toString() == "abc");
    TEST_ASSERT(stream.buffer().toString() == "abc");

    stream << "def" ;
    assert(stream.buffer().toString() == "abcdef");
    TEST_ASSERT(stream.buffer().toString() == "abcdef");

    stream << 100 ;
    assert(stream.buffer().toString() == "abcdef100");
    TEST_ASSERT(stream.buffer().toString() == "abcdef100");

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


  TestLoggerStream()
  {
    TEST_ADD(TestLoggerStream::test_append);
    TEST_ADD(TestLoggerStream::test_append_over);
    TEST_ADD(TestLoggerStream::test_close);
  }
};
 
int main()
{
  TestLoggerStream tl;
  Test::TextOutput output(Test::TextOutput::Verbose);
  (tl.run(output, false));
  return 0;
}

 