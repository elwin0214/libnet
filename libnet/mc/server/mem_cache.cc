#include <libnet/timestamp.h>
#include <libnet/mc/mem_cache.h>

namespace mc
{
namespace server
{
using namespace libnet;

void MemCache::remove(Item* item)
{
  hash_table_.removeItem(item);
  slab_array_.push(item);
};

Item* MemCache::find(const char* key, bool lru)
{
  Item* item = hash_table_.get(key);
  uint64_t now = Timestamp::now().secondsValue();
  if (NULL == item) return NULL;
  if (item->expired(now))
  {
    LOG_TRACE << "key = " << (item->key()) << " exptime = " << (item->get_exptime()) << " result = expired" ;
    lru_list_.remove(item);
    hash_table_.removeItem(item);
    slab_array_.push(item);
    return NULL;
  }
  if (lru)
  {
    lru_list_.access(item);
  }
  return item;
};

Item* MemCache::alloc(size_t data_size)
{
  int index = -1; 
  uint64_t now = Timestamp::now().secondsValue();
  Item* item = slab_array_.pop(data_size, index);
  if (item != NULL) 
    return item;

  if (index >= 0)
  {
    assert(index < 256);
    item = lru_list_.recycle(index, now);
    if (NULL != item) 
      hash_table_.removeItem(item);
    item->set_time(0);
    item->set_exptime(0);
  }else{
    LOG_ERROR << "data_size = " << data_size << " error = can not find";
  }
  return item;
};

void MemCache::add(Item* item)
{
  hash_table_.addItem(item);
  lru_list_.add(item);
};

}
}