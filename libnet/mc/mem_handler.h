#ifndef __LIBNET_MEMCACHED_SERVER_HANDLER_H__
#define __LIBNET_MEMCACHED_SERVER_HANDLER_H__
#include <libnet/nocopyable.h>
#include <libnet/mc/mem_cache.h>
#include <libnet/mc/message.h>
#include <functional>
#include <memory>

namespace mc
{
namespace server
{
using namespace mc::msg;

class MemHandler : public NoCopyable
{
public:
  typedef std::function<size_t(std::string)> HashFunc;

public:
  MemHandler(size_t shards, 
             HashFunc hash_func,
             size_t hashpower, 
             double factor,
             SlabOption option)
    : shards_(shards),
      locks_(shards_),
      hash_func_(hash_func),
      slablist_(option),
      caches_()
  {
    for (size_t i = 0; i <= shards_; i++)
      caches_.push_back(std::make_shared<MemCache>(hashpower, factor, hash_func_, slablist_));
  }

  MemHandler(size_t shards = 4, 
            HashFunc hash_func = std::function<size_t(std::string)>(),
            size_t hashpower = 8, 
            double factor = 1.2)
  : shards_(shards),
    locks_(shards_),
    hash_func_(hash_func),
    slablist_(SlabOption(16, 1024, 1.2, 1024 * 1024, true, 1024 * 1024 * 64)),
    caches_()
  {
    for (size_t i = 0; i <= shards_; i++)
      caches_.push_back(std::make_shared<MemCache>(hashpower, factor, hash_func_, slablist_));
  }

  void handle(Message& request, Message& response);

private:
  size_t shards_;
  std::vector<MutexLock> locks_;
  std::function<size_t(std::string)> hash_func_;
  ConcurrentSlabList slablist_;
  std::vector<std::shared_ptr<MemCache>> caches_;

};

}
}

#endif