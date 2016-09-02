#ifndef __LIBNET_MEMCACHED_SERVER_MEM_H__
#define __LIBNET_MEMCACHED_SERVER_MEM_H__
#include <sys/types.h>
#include <libnet/nocopyable.h>

namespace memcached
{
namespace server
{
using namespace libnet;
namespace detail
{
inline size_t align(size_t size)
{
  return (size + ((1 << 3) - 1)) & ~ ((1 << 3) -1);
};
}

class MemoryAllocator : public NoCopyable
{
public:
  MemoryAllocator(bool prealloc, size_t total);
  ~MemoryAllocator();
  
  void* allocate(size_t size);
  void deallocate(void* ptr);

private:
  bool prealloc_;
  size_t current_;
  size_t total_;
  char* ptr_;
};

}
}

#endif