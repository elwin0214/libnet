#include <libnet/inet_address.h>
#include <gtest/gtest.h>
#include <iostream>
#include <assert.h>

using namespace std;
using namespace libnet;

TEST(Address, convert)
{
  InetAddress addr("127.0.0.1", 9999);
  ASSERT_EQ(addr.getIp(), "127.0.0.1");
  ASSERT_EQ(addr.getPort(), 9999);
}

TEST(Address, copy)
{
  InetAddress addr("127.0.0.1", 9999);
  ASSERT_EQ(addr.getIp(), "127.0.0.1");
  ASSERT_EQ(addr.getPort(), 9999);

  InetAddress addr2 = addr;
  ASSERT_EQ(addr2.getIp(), "127.0.0.1");
  ASSERT_EQ(addr2.getPort(), 9999);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
