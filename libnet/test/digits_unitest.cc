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
  convert<uint32_t>(s.c_str(), value);
  ASSERT_EQ(value, 12);
}
 
TEST(digits, string2digit)
{
  size_t num;
  stringToDigit("16", &num);
  ASSERT_EQ(16, num);
}

TEST(digits, string2digit16)
{
  uint16_t num;
  uint32_t value  = 1 << 20;
  char buf[32];
  sprintf(buf, "%d", value);
  int r = stringToDigit(buf, &num);
  ASSERT_EQ(0, num);

}

TEST(digits, xstring2digit)
{
  size_t num;
  xstringToDigit("10", &num);
  ASSERT_EQ(16, num);
}

TEST(digits, digit2string)
{
  char buf[100];
  bzero(buf, sizeof(buf));
  digitToString(static_cast<uint32_t>(16), buf);
  ASSERT_EQ("16", std::string(buf, 2));
}

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