#include <list>
#include <assert.h>
#include <libnet/logger.h>
#include <libnet/exception.h>
#include <libnet/mc/slab.h>

#define ENTIRE_SIZE(size) (sizeof(Item) + size)
#define ALIGN(size) (((size) + ((8) - 1)) & ~ ((8) -1))

namespace mc
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


SlabList::SlabList(const SlabOption& option)
  : factor_(option.factor_),
    min_size_(option.min_size_),
    max_size_(option.max_size_),
    batch_alloc_size_(option.batch_alloc_size_),
    slabs_(),
    allocator_(option.prealloc_, option.total_mem_size_)
{
  init();
};

void SlabList::init()
{

  if (factor_ <= 1)
  {
    factor_ = 1.2;
  }
  min_size_ = ALIGN(ENTIRE_SIZE(min_size_));
  max_size_ = ALIGN(ENTIRE_SIZE(max_size_));
  if (batch_alloc_size_ < max_size_)
    batch_alloc_size_ = max_size_;
  LOG_DEBUG << " min_size_ = " << min_size_ 
            << " max_size_ = " << max_size_ 
            << " batch_alloc_size_ = " << batch_alloc_size_;
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
    last_size = max_size_;
    sizes_.push_back(max_size_);
  }
  max_size_ = last_size;
  slabs_.reserve(kMaxSlabs);

  size_t index = 0;
  bool need_max = false;
  for (auto size : sizes_)
  {
    if (index < kMaxSlabs - 1)
    {
      LOG_DEBUG << "index = " << index << " size = " << size ;
      slabs_.emplace_back(index, size);
      index++;
    }
    else
    {
      need_max = true;
      break;
    }
  }
  if (need_max){
    sizes_.push_back(max_size_);
    slabs_.emplace_back(index, max_size_);
    LOG_DEBUG << "index = " << index << " size = " << max_size_ ;
  }

  for (auto& slab : slabs_)
  {
    if (doAlloc(slab) <= 0)
    {
      LOG_SYSFATAL << " slab_index = " << (slab.index()) 
                   << " item_size = " << slab.item_size()
                   << " can not alloc " << batch_alloc_size_ << " bytes memory";
    }
  }
  //max_data_size_ = max_size_ - sizeof(Item);
};

size_t SlabList::doAlloc(Slab& slab)
{
  size_t item_size = slab.item_size();
  assert(item_size <= max_size_);

  char* ptr = static_cast<char*>(allocator_.allocate(batch_alloc_size_));

  if (ptr != NULL)
  {
    size_t offset = 0;
    size_t number = 0;
    for(; offset + item_size <= batch_alloc_size_; )
    {
      void* item_ptr = static_cast<void*>(ptr + offset);
      Item* item = new (item_ptr)Item(slab.index(), item_size - sizeof(Item));
      offset += item_size;
      slab.push(item);
      number++;
    }
    LOG_TRACE << " slab_index = " << (slab.index()) 
              << " item_size = " << item_size 
              << " batch_size = " << batch_alloc_size_
              << " number = " << number;
    return number;
  }
  else
  {
    LOG_ERROR << " slab_index = " << (slab.index()) 
              << " item_size = " << item_size 
              << " can not alloc " << batch_alloc_size_ << " bytes memory";
    return 0;
  }
};

Item* SlabList::pop(size_t data_size, int& index)
{ 

  size_t item_size = ENTIRE_SIZE(data_size);
  int i = -1;
  bool hit = false;
  for (int size : sizes_)
  {
    i++;
    if (size >= item_size)
    {
      item_size = size;
      hit = true;
      break;
    }
  }
  if (!hit)
  {
    index = -1;
    return NULL;
  }
  
  Slab& slab = slabs_[i];
  if (slab.number() == 0)
  {
    doAlloc(slab);
  }
  Item* item = slab.pop();
  index = i;
  return item;
};
  
void SlabList::push(Item* item)
{
  slabs_[item->index()].push(item); 
};

}
}