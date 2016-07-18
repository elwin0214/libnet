#include <libnet/digits.h>
#include <iostream>
#include <assert.h>
#include <strings.h>
#include <string>

using namespace libnet::digits;
using namespace std;

int main()
{
  int num;
  stringToDigit("16", &num);
  assert(16 == num);
  cout << num << endl;


  int num2;
  xstringToDigit("10", &num2);
  assert(16 == num);
  cout << num2 << endl;


  char buf[100];
  bzero(buf, sizeof(buf));
  digitToString(16, buf);
  assert("16" == std::string(buf, 2));
  cout << buf << endl;

  char buf2[100];
  digitToXstring(16, buf2);
  assert("10" == std::string(buf2, 2));
  cout << buf2 << endl;



  // int s = convert("16", 10);
  // cout << s << endl;
  return 0;
}