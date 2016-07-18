#include <iostream>
#include <assert.h>
#include "../endian_ops.h"

using namespace libnet;
using namespace libnet::sockets;

using namespace std;

void test()
{
  for (int i = 0; i < 30 ; i++)
  {
    uint32_t n = 1 << i;
    assert(n == network32ToHost(hostToNetwork32(n)));
  }  
};

int main()
{
  test();
  return 0;
}