#include "../tokenizer.h"
#include <libnet/buffer.h>
#include <gtest/gtest.h>
#include <string>
#include <assert.h>

using namespace std;
using namespace libnet;
using namespace memcached::server;

TEST(Tokenizer, incr)
{
  Buffer buffer;
  buffer.append("  incr key 1\r\n");
  const char* end = buffer.find("\r\n");
  Tokenizer tokenizer(buffer.beginRead(), end, ' ');

  const char* pos = NULL;
  size_t len = 0;
  bool next = tokenizer.next(pos, len);
  if (next)
  {
    ASSERT_EQ(std::string(pos, len), "incr");
  }
}

TEST(Tokenizer, nonext)
{
  Buffer buffer;
  buffer.append(" \r\n");
  const char* end = buffer.find("\r\n");
  Tokenizer tokenizer(buffer.beginRead(), end, ' ');

  const char* pos = NULL;
  size_t len = 0;
  bool next = tokenizer.next(pos, len);
  ASSERT_TRUE(!next);
}

TEST(Tokenizer, set)
{
  Buffer buffer;
  buffer.append(" set a 0 0 1\r\n");
  const char* end = buffer.find("\r\n");
  Tokenizer tokenizer(buffer.beginRead(), end, ' ');

  const char* pos = NULL;
  size_t len = 0;
  bool next = false;
  next = tokenizer.next(pos, len);
  ASSERT_EQ(std::string(pos, len), "set");

  next = tokenizer.next(pos, len);
  ASSERT_EQ(std::string(pos, len), "a");

  next = tokenizer.next(pos, len);
  ASSERT_EQ(std::string(pos, len), "0");
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}