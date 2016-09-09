#include <libnet/digits.h>
#include <iostream>
#include <assert.h>
#include <strings.h>
#include <string>

using namespace libnet::digits;
using namespace std;

void test_convert()
{
  std::string s = "12";
  uint32_t value = 0;
  convert<uint32_t>(s.c_str(), value);
  assert(value == 12);
};

void test_string_digit()
{
  size_t num;
  stringToDigit("16", &num);
  assert(16 == num);
  cout << num << endl;
};

void test_string_digit_width()
{
  uint16_t num;
  uint32_t value  = 1 << 20;
  char buf[32];
  sprintf(buf, "%d", value);
  cout << buf  << endl;
  int r = stringToDigit(buf, &num);
  //assert(16 == num);
  cout << "result = " << r <<" num = "<< num << endl;
};


void test_xstring_digit()
{
  size_t num;
  xstringToDigit("10", &num);
  assert(16 == num);
  cout << num << endl;
};

void test_digit_string()
{
  char buf[100];
  bzero(buf, sizeof(buf));
  digitToString(static_cast<uint32_t>(16), buf);
  assert("16" == std::string(buf, 2));
  cout << buf << endl;
};

void test_digit_xstring()
{
  char buf[100];
  digitToXstring(16, buf);
  assert("10" == std::string(buf, 2));
  cout << buf << endl;
}

int main()
{
  test_convert();
  test_string_digit();
  test_string_digit_width();
  test_xstring_digit();
  test_digit_string();
  test_digit_xstring();
  return 0;
}