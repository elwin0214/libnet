#include <iostream>
#include <vector>
#include <libnet/logger.h>
#include <libnet/buffer.h>
#include <assert.h>
#include <gtest/gtest.h>

using namespace std;
using namespace libnet;

TEST(Buffer, append)
{
  Buffer buf(0, 10);
  buf.append("abc", 3);
  ASSERT_EQ(string(buf.cur(), buf.readable()) , "abc");
}

TEST(Buffer, appendOver)
{
  Buffer buf(0, 2);
  buf.append("abc", 3);
  ASSERT_EQ(string(buf.cur(), buf.readable()) , "abc");
  ASSERT_TRUE(buf.capcity() >= 3);
}

TEST(Buffer, move)
{
  Buffer buf(0, 2);
  buf.append("abc", 3);
  buf.moveReadIndex(2);
  ASSERT_EQ(string(buf.cur(), buf.readable()), "c");

  buf.append("de", 2);
  ASSERT_EQ(string(buf.cur(), buf.readable()), "cde");

  ASSERT_TRUE((buf.readIndex()) == 0);
}

TEST(Buffer, equal)
{
  Buffer buf(0, 2);
  buf.append("exit\r\n", 6);

  ASSERT_TRUE(buf.equals("exit\r\n"));
  ASSERT_TRUE(!buf.equals("abcd"));
  ASSERT_TRUE(!buf.equals("ab"));
}

TEST(Buffer, find)
{
  Buffer buf(0, 100);
  const char *s = "GET /index HTTP/1.1\r\n";
  buf.append(s, strlen(s));
  const char *p = buf.find("\r\n");
  ASSERT_TRUE((*p == '\r'));
}

TEST(Buffer, find_from)
{
  Buffer buf(0, 100);
  const char *s = "abc\r\n123\r\n";
  buf.append(s, strlen(s));
  const char *p = buf.find(5, "\r\n");
  ASSERT_TRUE((*p == '\r'));
}

TEST(Buffer, prepare)
{
  Buffer buf(16, 1024);
  buf.append("0123456789", 10);
  buf.prepare("abc", 3);
  ASSERT_TRUE(("abc0123456789" == buf.toString()));
}

TEST(Buffer, startWiths)
{
  Buffer buf(16, 1024);
  buf.append("0123456789", 10);
  ASSERT_TRUE(buf.startWiths("0123"));
  ASSERT_TRUE(buf.startWiths("0123", 4));
  ASSERT_TRUE(buf.startWiths("0123", 2));
  ASSERT_TRUE(!buf.startWiths("2123", 2));
}

TEST(Buffer, ascii)
{
  Buffer buf(16, 1024);
  buf.append("\r\n");
  ASSERT_EQ("[13][10]", buf.toAsciiString());
}

TEST(Buffer, makeRoom1)
{
  Buffer buffer(4, 8);
  buffer.append("12345678");
  buffer.moveReadIndex(5);
  buffer.makeRoom(4);
  ASSERT_EQ("678", buffer.toString());
}

TEST(Buffer, makeRoom2)
{
  Buffer buffer(4, 8);
  buffer.append("12345678");
  buffer.makeRoom(4);
  ASSERT_EQ("12345678", buffer.toString());
}

TEST(Buffer, appendOver2)
{
  
  Buffer buffer(0, 4096);
  for (int i = 0; i < 200; i++)
  {
    buffer.append("set key-5846 0 0 10");
    buffer.append("\r\n");
    buffer.append("1111111111");
    buffer.append("\r\n");
  }
  cout << buffer.readable() << endl;
  cout << buffer.toString() ;
  LOG_INFO << buffer.toAsciiString() << " end";
 // ASSERT_EQ("12345678", );
}
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

