#include <libnet/mc/lru.h>

namespace mc
{
namespace server
{

void LRUList::remove(Item* item)
{
  size_t index = item->index();
  if (item->prev_ == NULL)// head node
  {
    head_item_list_[index] = item->next_;
    if (item->next_ != NULL)
      item->next_->prev_ = NULL;
    else
      tail_item_list_[index] = NULL; //tail node
    
  }
  else
  {
    item->prev_->next_ = item->next_;
    if (item->next_ != NULL)
      item->next_->prev_ = item->prev_;
    else
      tail_item_list_[index] = item->prev_; //tail node
  }
  nums_list_[index]--;

  //item->set_time(0);
  //item->set_exptime(0);
};

void LRUList::access(Item* item)
{
  remove(item);
  add(item);
};

void LRUList::add(Item* item)
{
  size_t index = item->index();
  item->next_ = head_item_list_[index];
  head_item_list_[index] = item;
  item->prev_ = NULL;
  if (item->next_ == NULL)
  {
    assert(tail_item_list_[index] == NULL);
    tail_item_list_[index] = item;
  }
  else
  {
    item->next_->prev_ = item;
  }
  nums_list_[index]++;
};

// Find and remove the item which is expired. 
// If can not find, remove the last item. 
Item* LRUList::recycle(size_t index, uint64_t now)
{
  Item* item = tail_item_list_[index];
  if (NULL == item)
  {
    return NULL;
  }
  int number = 0; 
  for (; NULL != item && number < 3; item = item->prev_) 
  { 
    number++;
    // dont remove the item whose exptime is 0
    // may slow the request if all item exptime is 0.
    if (item->exptime_ == 0) continue; 
    if (item->exptime_ < now)
    {
      remove(item);
      return item;
    }
  }
  item = tail_item_list_[index];
  remove(item);
  return item;
};

}
}