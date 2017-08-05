#ifndef __LIBNET_MEMCACHED_SERVER_MEM_H__
#define __LIBNET_MEMCACHED_SERVER_MEM_H__
#include <sys/types.h>
#include <libnet/nocopyable.h>

namespace mc
{
namespace server
{
using namespace libnet;

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