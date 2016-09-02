#include <list>
#include <assert.h>
#include "slab.h"
#include <libnet/logger.h>

namespace memcached
{
namespace server
{
static const size_t batch_alloc_size = 1024 ;

Item* Slab::pop()
{
  if (head_ == NULL) return NULL;

  Item* tmp = head_;
  Item* next = head_->next();
  head_= next;
  if (tail_ == tmp) tail_ = NULL;
  number_--;
  return tmp; 
};

void Slab::push(Item* item)
{
  if (head_ == NULL)
  {
    head_ = item;
    tail_ = item;
    head_->next_ = NULL;
    number_++;
    return;
  }

  tail_->next_ = item;
  tail_ = item; 
  tail_->next_ = NULL;
  number_++;
};


SlabArray::SlabArray(SlabPolicy policy)
  : factor_(policy.factor_),
    min_size_(policy.item_min_size_),
    max_size_(policy.item_max_size_),
    slabs_(),
    allocator_(new MemoryAllocator(policy.prealloc_, policy.total_mem_size_))
{

};

void SlabArray::init()
{
  if (factor_ <= 1)
  {
    factor_ = 1.2;
  }
  std::list<int> sizes_;
  size_t size;
  size_t last_size;
  for (size = detail::align(min_size_); size <= max_size_; )
  {
    sizes_.push_back(size);
    last_size = size;
    size *= factor_; 
    size = detail::align(size);
  }
  if (last_size < max_size_)
  {
    size = detail::align(max_size_);
    sizes_.push_back(size);
  }
  slabs_.reserve(sizes_.size());

  int index = 0;
  for (std::list<int>::iterator itr = sizes_.begin();
      itr != sizes_.end();
      itr++)
  {
    LOG_DEBUG << "index = " << index << " size = " << (*itr) ;
    slabs_.push_back(Slab(index, *itr));
    index++;
  }
};

void SlabArray::doAlloc(Slab& slab, size_t item_size)
{
  size_t item_obj_size = sizeof(Item);
  size_t item_entire_size = item_obj_size + item_size;

  assert(item_entire_size <= batch_alloc_size);

  char* ptr = static_cast<char*>(allocator_->allocate(batch_alloc_size));

  if (ptr != NULL)
  {
    Item* item = NULL;
    size_t offset = 0;
    for(; offset + item_entire_size <= batch_alloc_size; )
    {
      item = static_cast<Item*>(static_cast<void*>(ptr + offset));
      item->clear();
      item->reset(item_size);
      offset += item_entire_size;
      slab.push(item);
    }

    if (offset + item_entire_size < batch_alloc_size )
    {
      item->reset(batch_alloc_size - offset);//last item get more space
    }
  }

};

Item* SlabArray::pop(size_t item_size)
{
  if (item_size > max_size_) return NULL;

  for (std::vector<Slab>::iterator itr = slabs_.begin();
      itr != slabs_.end();
      itr++)
  {
    if (itr->item_size() >= item_size)
    {
      if (itr->number() == 0)
      {
        doAlloc(*itr, item_size);
      }
      return itr->pop();
    }
  }
  assert(false);
  return NULL;
};
  
void SlabArray::push(Item* item)
{
  slabs_[item->index()].push(item); 
};

}
}