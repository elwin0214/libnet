#include <gtest/gtest.h>
#include <libnet/mc/allocator.h>

using namespace mc::server;
using namespace std;

TEST(Allocator, alloc)
{
  MemoryAllocator allocator(true, 1024 * 1024);
  char* last = NULL;
  for (int i = 0; i < 1024; i++)
  {
    char* p = static_cast<char*>(allocator.allocate(1024));
    ASSERT_TRUE(NULL != p);
    if (NULL != last)
    { 
      ASSERT_EQ(p - 1024 , last);
      last = p;
    }
  }
  char* p = static_cast<char*>(allocator.allocate(1024));
  ASSERT_TRUE(NULL == p);
}

TEST(Allocator, dealloc)
{
  char* last = NULL;
  for (int i = 0; i < 1024; i++)  //1T memory 
  {
    MemoryAllocator allocator(true, 1024 * 1024 * 1024);
    char* p = static_cast<char*>(allocator.allocate(1024));
    ASSERT_TRUE(NULL != p);
  }
}



int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}