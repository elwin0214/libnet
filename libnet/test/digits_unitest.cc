#include <libnet/digits.h>
#include <gtest/gtest.h>
#include <iostream>
#include <assert.h>
#include <strings.h>
#include <string>

using namespace libnet::digits;
using namespace std;


TEST(Digits, convert)
{
  std::string s = "12";
  uint32_t value = 0;
  convert<uint32_t>(s.c_str(), value, 10);
  ASSERT_EQ(value, 12);
}

TEST(Digits, overflow)
{
  long long l = 1;
  l = (l << 63) - 1;
  std::string str = to_string(l);
  uint32_t value = 0;
  bool result = convert<uint32_t>(str.c_str(), value, 10);
  ASSERT_TRUE(!result);
}

TEST(Digits, error_format)
{
  
  std::string str = "1abc2";
  uint32_t value = 0;
  bool result = convert<uint32_t>(str.c_str(), value, 10);
  ASSERT_TRUE(!result);
}
   
TEST(Digits, digit2xstring)
{
  char buf[100];
  digitToXstring(16, buf);
  ASSERT_EQ("10", std::string(buf, 2));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}