#include <stdlib.h>
#include "allocator.h"

namespace memcached
{
namespace server
{



MemoryAllocator::MemoryAllocator(bool prealloc, size_t total)
  : prealloc_(prealloc),
    current_(0),
    total_(total),
    ptr_(NULL)
{
  total_ = detail::align(total_);
  if (prealloc_)
    ptr_ = static_cast<char*>(::malloc(total_));
};

MemoryAllocator::~MemoryAllocator()
{
  if (prealloc_)
    ::free(ptr_);
};

void* MemoryAllocator::allocate(size_t size)
{
  size = detail::align(size);
  if (!prealloc_)
  {
    return static_cast<char*>(::malloc(size));
  }
  if (ptr_ == NULL)
  {
    ptr_ = static_cast<char*>(::malloc(total_));
  }
  if (current_ + size > total_)
    return NULL;
  char* result = ptr_ + current_;
  current_ += size;
  return static_cast<void*>(result);
};

void MemoryAllocator::deallocate(void* ptr)
{
  if(!prealloc_)
    ::free(ptr);
};

}
}
