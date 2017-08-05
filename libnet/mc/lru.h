#ifndef __LIBNET_MC_SERVER_LRU_H__
#define __LIBNET_MC_SERVER_LRU_H__

#include <vector>
#include <assert.h>
#include "item.h"

namespace mc
{
namespace server
{

class LRUList
{
public:
  LRUList(size_t size)
    : head_item_list_(size, NULL),
      tail_item_list_(size, NULL),
      nums_list_(size, 0)
  {

  }
  
  void remove(Item* item);

  void access(Item* item);

  void add(Item* item);

  Item* recycle(size_t index, uint64_t now);

  Item* head(size_t index) { return head_item_list_[index]; }

  Item* tail(size_t index) { return tail_item_list_[index]; }

  size_t number(size_t index) { return nums_list_[index]; } 

private:
  std::vector<Item*> head_item_list_;
  std::vector<Item*> tail_item_list_;
  std::vector<size_t> nums_list_;
};

}
}

#endif