#include <assert.h>
#include <gtest/gtest.h>
#include <string.h>
#include <string>
#include <libnet/mc/reader.h>

using namespace std;
using namespace mc::msg;

TEST(Reader, read)
{
  const char* p = "a b cc dd";
  Reader reader(p, strlen(p) + p, ' ');
  string value;
  ASSERT_TRUE(reader.read(value));
  ASSERT_EQ("a", value);
}  


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}