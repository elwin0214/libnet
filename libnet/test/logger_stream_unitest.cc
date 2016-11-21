#include <libnet/logger.h>
#include <gtest/gtest.h>
#include <assert.h>

using namespace libnet;
using namespace libnet::log;

TEST(LoggerStream, append)
{
  LoggerStream stream;
  stream << "abc" ;
  ASSERT_EQ(stream.buffer().toString(), "abc");

  stream << "def" ;
  ASSERT_EQ(stream.buffer().toString(), "abcdef");

  stream << 100 ;
  ASSERT_EQ(stream.buffer().toString(), "abcdef100");
}

TEST(LoggerStream, append_over)
{
  LoggerStream stream;
  for (int i = 0 ; i <= 2000 ; i++)
  {
    stream << "a" ;
  }
  stream << "abc" ;
  stream << "def" ;
  ASSERT_EQ(stream.buffer().remain(), 0);
  ASSERT_EQ(stream.buffer().size(), 1024);
}

TEST(LoggerStream, close)
{
  LoggerStream stream;
    
  stream << "abc" ;
  stream.close();
  stream << "def" ;
  ASSERT_EQ(stream.buffer().toString(), "abc");
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}