#include <string.h>
#include <assert.h>
#include <iostream>
#include <map>
#include "../processor.h"

using namespace std;
using namespace libnet;
using namespace memcached::server;
using namespace std::placeholders;

struct ProcessorProxy
{
  ProcessorProxy(): cache_(),processor_(),buffer_(NULL)
  {
    processor_.set_item_find(std::bind(&ProcessorProxy::find, this, _1));
    processor_.set_send(std::bind(&ProcessorProxy::send, this, _1));
    processor_.set_item_alloc(std::bind(&ProcessorProxy::alloc, this, _1));
    processor_.set_item_add(std::bind(&ProcessorProxy::add, this, _1));
    processor_.set_item_remove(std::bind(&ProcessorProxy::remove, this, _1));

    processor_.init();
  };


  void process(Buffer& req, Buffer& resp, MemcachedContext& context)
  {
    buffer_ = &resp;
    processor_.process(req, context);
  }

  Item* alloc(uint32_t size)
  {
    return static_cast<Item*>(::malloc(sizeof(Item) + size));// will not free
  };

  void remove(Item* item)
  {
    const char* key = item->key();
    auto itr = cache_.find(key);
    assert(itr != cache_.end());
    assert((itr->second) == item);
    cache_.erase(itr);
  }

  void add(Item* item)
  {
    const char* key = item->key();
    cache_[key] = item;
  }

  Item* find(const char* key)
  {
    auto itr = cache_.find(key);
    if (itr == cache_.end()) 
      return NULL;
    return itr->second;
  };

  void send(const char* str)
  {
    buffer_->append(str);
  };

  void clear()
  {
    cache_.clear();
  };

  std::map<std::string, Item*> cache_;
  MemcachedProcessor processor_;
  Buffer* buffer_;

};

ProcessorProxy gProcessor;


void process(const char* input, const char* ouput, Opt opt)
{
  MemcachedContext context;
  Buffer req(0, 1024);
  Buffer resp(0, 1024);
  req.append(input);
  gProcessor.process(req, resp, context);
  cout << "resp = " << (resp.toString())  << endl;
  assert(resp.toString() == ouput);
  assert(opt == context.get_opt());
};

void test_single_opt()
{
  gProcessor.clear();
  process(" get key1\r\n" ,"END\r\n", kNo);
  process(" get key1\r\n" ,"END\r\n", kNo);

  process(" set a 0 0 1\r\n1\r\n" ,"STORED\r\n", kNo);
  process(" get a\r\n" ,"VALUE a 0 1\r\n1\r\nEND\r\n", kNo);

  process(" incr a 1\r\n", "2\r\n", kNo);
  process(" decr a 1\r\n", "1\r\n", kNo);

  process(" replace b 0 0 2\r\n23\r\n" ,"NOT_STORED\r\n", kNo);
  process(" add b 0 0 2\r\n23\r\n" ,"STORED\r\n", kNo);

  process(" delete c\r\n" ,"NOT_FOUND\r\n", kNo);
  process(" delete b\r\n" ,"DELETED\r\n", kNo);

}

void test_multi_opt()
{
  gProcessor.clear();
  process(" get key1\r\nget b" ,"END\r\n", kNo);
  process(" get key1\r\nget b\r\n" ,"END\r\nEND\r\n", kNo);
  
  process(" add b 0 0 2\r\n23" ,"", kAdd);
  process(" add b 0 0 2\r\n23\r\nget b\r\n" ,"STORED\r\nVALUE b 0 2\r\n23\r\nEND\r\n", kNo);
}


int main()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  test_single_opt();
  test_multi_opt();
  // test_process("key", "value", " get key\r\n" ,"VALUE key 0 5\r\nvalue\r\nEND\r\n", kNo);
  // test_process("key", "value", 
  //   " get key\r\nget key\r\n" ,"VALUE key 0 5\r\nvalue\r\nEND\r\nVALUE key 0 5\r\nvalue\r\nEND\r\n",
  //   kNo);






  return 0;
}