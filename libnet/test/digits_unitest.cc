#include <libnet/digits.h>
#include <gtest/gtest.h>
#include <iostream>
#include <assert.h>
#include <strings.h>
#include <string>

using namespace libnet::digits;
using namespace std;


TEST(digits, convert)
{
  std::string s = "12";
  uint32_t value = 0;
  convert<uint32_t>(s.c_str(), value, 10);
  ASSERT_EQ(value, 12);
}
/*
TEST(digits, convert_string)// error
{
  std::string s = "12";
  uint32_t value = 0;
  convert<uint32_t>(s, value, 10);
  ASSERT_EQ(value, 12);
}*/
   
TEST(digits, digit2xstring)
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