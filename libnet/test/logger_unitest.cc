#include <libnet/logger.h>
#include <libnet/current_thread.h>
#include <gtest/gtest.h>
#include <assert.h>

using namespace std;
using namespace libnet;
using namespace libnet::log;


size_t append(char* buf, size_t size, const char* str)
{
  size_t r = snprintf(buf, 8, "%s", str); 
  if (r > size - 1) return size - 1;
  return r;
}

TEST(Logger, printf)
{
  char buf[8]; 
  string s = "123456789";
  int size = snprintf(buf, 8, "%s", s.c_str()); // size 不表示实际写入
  ASSERT_EQ(size, s.size());
  char ch = buf[7];
  ASSERT_EQ(ch, '\0');
  ASSERT_EQ(std::string(buf), "1234567");  
}


TEST(Logger, append)
{
  char buf[8];
  ASSERT_EQ(append(buf, 8, "123456789"), 7);
  ASSERT_EQ(*(buf + 7), '\0');

  char buf2[8];
  ASSERT_EQ(append(buf2, 8, "12345678"), 7);
  ASSERT_EQ(*(buf2 + 7), '\0');

  char buf3[8];
  ASSERT_EQ(append(buf3, 8, "123456"), 6);
  ASSERT_EQ(*(buf3 + 6), '\0');
}


TEST(Logger, info)
{

  char buf[2048];

  for (int i = 0; i < 2048; i++)
  {
    buf[i] = '1';
  }

  LOG_INFO << buf ;
}

 
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}