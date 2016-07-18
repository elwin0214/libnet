#include <libnet/inet_address.h>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace libnet;

void test_setter()
{
  InetAddress addr("127.0.0.1", 9999);
  assert(addr.getIp() == "127.0.0.1");
  assert(addr.getPort() == 9999);
}

int main()
{
  test_setter();
  return 0;
}

