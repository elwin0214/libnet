#ifndef __LIBNET_MC_SERVER_SLAB_H__
#define __LIBNET_MC_SERVER_SLAB_H__

#include <sys/types.h>
#include <vector>
#include <libnet/nocopyable.h>
#include <libnet/logger.h>
#include <libnet/mutexlock.h>
#include <libnet/mc/allocator.h>
#include <libnet/mc/item.h>

namespace mc
{
namespace server
{
using namespace libnet;

struct SlabOption
{ 
  SlabOption(size_t min_size, 
      size_t max_size, 
      double factor, 
      size_t batch_alloc_size, 
      bool prealloc,
      size_t total_mem_size)
    : min_size_(min_size),
      max_size_(max_size),
      factor_(factor),
      batch_alloc_size_(batch_alloc_size),
      prealloc_(prealloc),
      total_mem_size_(total_mem_size)
  {

  };

  size_t min_size_;  // item最小值
  size_t max_size_; //item最大值
  double factor_;  //增长因子
  size_t batch_alloc_size_;
  bool prealloc_;  //是否预先分配内存
  size_t total_mem_size_; //所需的总内存大小
};

// vector.reserve 扩容时候,Slab必须有copy构造，这里使用移动构造
class Slab : public NoCopyable
{

public:
  Slab(uint8_t index, size_t item_size)
    : number_(0),
      index_(index),
      item_size_(item_size),
      head_(NULL),
      tail_(NULL)
  {

  }

  Slab(Slab&& slab) noexcept
    : number_(slab.number_),
      index_(slab.index_),
      item_size_(slab.item_size_),
      head_(slab.head_),
      tail_(slab.tail_)
  {
    head_ = NULL;
    tail_ = NULL;
  }

  ~Slab() noexcept {}

  bool empty(){ return head_ == NULL; }

  Item* pop();

  void push(Item* item);

  size_t item_size() { return item_size_; }

  size_t number() { return number_; }
  
  uint8_t index() { return index_; }

private:
  size_t number_;
  uint8_t index_;
  size_t item_size_;
  Item* head_;
  Item* tail_;

};

class SlabList : public NoCopyable
{

public:
  SlabList(const SlabOption& option);

  Item* pop(size_t data_size, int& index);
  
  void push(Item* item);

  Slab& operator[](size_t index) { return slabs_[index]; }

  size_t size() { return slabs_.size(); }
    
private:
  void init();
  size_t doAlloc(Slab& slab);

private:
  double factor_;
  size_t min_size_;
  size_t max_size_;
  size_t batch_alloc_size_;
  std::vector<size_t> sizes_;
  std::vector<Slab> slabs_;
  MemoryAllocator allocator_;
};

class ConcurrentSlabList
{
public:
  ConcurrentSlabList(const SlabOption& option)
    : slablist_(option),
      size_(slablist_.size()),
      lock_()
  {

  }

  Item* pop(size_t data_size, int& index)
  {
    LockGuard guard(lock_);
    return slablist_.pop(data_size, index);
  }
  
  void push(Item* item)
  {
    LockGuard guard(lock_);
    slablist_.push(item);
  }

  size_t size() const
  {
    return size_;
  }

private:
  SlabList slablist_;
  size_t size_;
  MutexLock lock_;
};
//class Slabs

}
}

#endif