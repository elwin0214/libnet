#include "../buffer.cc"
#include "../cstring.h"
#include <iostream>

using namespace libnet;
using namespace std;

void test_find()
{
  Buffer buf(100);
  const char *s = "GET /index HTTP/1.1\r\n2";
  buf.append(s, strlen(s));
  cout << buf.toString() << endl;
  const char *p = buf.find("\r\n");
   assert((*p == '\r'));
};


int main()
{
  test_find();

  return 0;
}