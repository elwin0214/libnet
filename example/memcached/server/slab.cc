#include <list>
#include <assert.h>
#include "slab.h"
#include <libnet/logger.h>
#include <libnet/exception.h>

#define ENTIRE_SIZE(size) (sizeof(Item) + size)
#define ALIGN(size) (((size) + ((8) - 1)) & ~ ((8) -1))

namespace memcached
{
namespace server
{
static const size_t kMaxSlabs = 256;

Item* Slab::pop()
{
  if (head_ == NULL) return NULL;

  Item* tmp = head_;
  Item* next = head_->next_;
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
  assert(NULL != tail_);
  assert(NULL != head_);

  tail_->next_ = item;
  tail_ = item; 
  tail_->next_ = NULL;
  number_++;
};


SlabArray::SlabArray(const SlabOption& option)
  : factor_(option.factor_),
    min_size_(option.item_min_size_),
    max_size_(option.item_max_size_),
    batch_alloc_size_(option.batch_alloc_size_),
    slabs_(),
    allocator_(option.prealloc_, option.total_mem_size_)
{
  init();
};

void SlabArray::init()
{

  if (factor_ <= 1)
  {
    factor_ = 1.2;
  }
  min_size_ = ALIGN(ENTIRE_SIZE(min_size_));
  max_size_ = ALIGN(ENTIRE_SIZE(max_size_));
  std::list<int> sizes_;
  size_t size;
  size_t last_size;
  for (size = min_size_; size <= max_size_; )
  {
    sizes_.push_back(size);
    last_size = size;
    size *= factor_; 
    size = ALIGN(size);
  }
  if (last_size < max_size_)
  {
    //size = (max_size_);
    last_size = max_size_;
    sizes_.push_back(max_size_);
  }
  max_size_ = last_size;
  slabs_.reserve(kMaxSlabs);

  size_t index = 0;
  bool need_max = false;
  for (std::list<int>::iterator itr = sizes_.begin();
      itr != sizes_.end();
      itr++)
  {
    if (index < kMaxSlabs - 1)
    {
      LOG_DEBUG << "index = " << index << " size = " << (*itr) ;
      slabs_.push_back(Slab(index, *itr));
      index++;
    }
    else
    {
      need_max = true;
      break;
    }
  }
  slabs_.push_back(Slab(index, max_size_));
  max_item_size_ = max_size_ - sizeof(Item);
};

void SlabArray::doAlloc(Slab& slab, size_t item_entire_size)
{
  assert(item_entire_size <= max_size_);

  char* ptr = static_cast<char*>(allocator_.allocate(batch_alloc_size_));

  if (ptr != NULL)
  {
    size_t offset = 0;
    for(; offset + item_entire_size <= batch_alloc_size_; )
    {

      void* item_ptr = static_cast<void*>(ptr + offset);
      Item* item = new (item_ptr)Item(slab.index(), item_entire_size - sizeof(Item));
      offset += item_entire_size;
      LOG_TRACE << "slab.index = " << (slab.index()) << "item->size()= " << (item->size()) << " " << (item->size_) ;
      slab.push(item);
    }
  }
  else
    LOG_ERROR << "can not alloc " << batch_alloc_size_ << " bytes memory";

};

Item* SlabArray::pop(size_t item_size, int& index)
{ 

  size_t item_entire_size = ENTIRE_SIZE(item_size);
  if (item_entire_size > max_size_) return NULL;

  for (std::vector<Slab>::iterator itr = slabs_.begin();
      itr != slabs_.end();
      itr++)
  {
    if (itr->item_size() >= item_entire_size)
    {
      if (itr->number() == 0)
      {
        doAlloc(*itr, itr->item_size());
      }
      Item* item = itr->pop();
      if (NULL == item)
      {
        return NULL;
      }
      index = item->index();
      return item;
    }
  }
  index = -1;
  //throw Exception("can not find matched item in the slab!");
  assert(false);
  return NULL;
};
  
void SlabArray::push(Item* item)
{
  slabs_[item->index()].push(item); 
};

}
}