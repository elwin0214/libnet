#include <iostream>
#include <assert.h>
#include <gtest/gtest.h>
#include "../endian_ops.h"

using namespace libnet;
using namespace libnet::sockets;

using namespace std;

TEST(endian, convert)
{
 for (int i = 0; i < 30 ; i++)
  {
    uint32_t n = 1 << i;
    ASSERT_EQ(n, network32ToHost(hostToNetwork32(n)));
  }  
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}