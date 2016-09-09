#ifndef __LIBNET_MEMCACHED_SERVER_PROCESSOR_H__
#define __LIBNET_MEMCACHED_SERVER_PROCESSOR_H__
#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <functional>
#include <memory>
#include <libnet/buffer.h>
#include <libnet/digits.h>
#include "item.h"
#include "tokenizer.h"
#include "memcached_context.h"


namespace memcached
{
namespace server
{
using namespace libnet;
class Item;
class MemcachedProcessor;

class Processor
{
public:
  Processor(Opt opt)
    :opt_(opt)
  {

  };

  bool isDigit(const char* str);
  virtual bool process(Buffer& buffer, MemcachedContext& context)=0;


protected:
  friend class MemcachedProcessor;
  Opt opt_;
  std::function<Item*(const char*, bool lru)> item_find_func_;
  std::function<void(Item*)> item_remove_func_;
  std::function<Item*(uint32_t)> item_alloc_func_;
  std::function<void(Item*)> item_add_func_;
};


class MemcachedProcessor
{

public:
  MemcachedProcessor();

  void init();
  
  void process(Buffer& buffer, MemcachedContext& context);

  void set_item_find(std::function<Item*(const char*, bool)> func) 
  {
    item_find_func_ = func;
  }

  void set_item_remove(std::function<void(Item*)> func)
  {
    item_remove_func_ = func;
  }

  void set_item_alloc(std::function<Item*(uint32_t)> func)
  {
    item_alloc_func_ = func;
  }

  void set_item_add(std::function<void(Item*)> func)
  {
    item_add_func_ = func;
  }

private:
  std::vector<std::shared_ptr<Processor>> processors_; //virtual deconstrutor call

  std::function<Item*(const char*, bool)> item_find_func_;
  std::function<void(Item*)> item_remove_func_;
  std::function<Item*(uint32_t)> item_alloc_func_;
  std::function<void(Item*)> item_add_func_;
};

}
}

#endif