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
  ProcessorProxy(): cache_(),processor_()
  {
    processor_.set_item_find(std::bind(&ProcessorProxy::find, this, _1));
    processor_.set_item_alloc(std::bind(&ProcessorProxy::alloc, this, _1));
    processor_.set_item_add(std::bind(&ProcessorProxy::add, this, _1));
    processor_.set_item_remove(std::bind(&ProcessorProxy::remove, this, _1));
    processor_.init();
  };

  void process(Buffer& req, MemcachedContext& context)
  {
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

  void clear()
  {
    cache_.clear();
  };

  std::map<std::string, Item*> cache_;
  MemcachedProcessor processor_;
};

ProcessorProxy gProcessor;


void process(const char* input, const char* ouput, Opt opt)
{
  MemcachedContext context;
  bool closed = false;
  Buffer resp(0, 1024);
  Buffer req(0, 1024);
  context.set_send_func([&resp] (const char* str){ resp.append(str);});
  context.set_close_func([&closed] (){ closed = true;});
  req.append(input);
  gProcessor.process(req, context);
  cout << "closed = " << closed << " resp = " << (resp.toString()) << endl;
  assert(resp.toString() == ouput);
  assert(opt == context.get_opt());
};

void test_single_opt()
{
  gProcessor.clear();
  process(" get key1\r\n" ,"END\r\n", kNo);

  process(" set a 0 0 1\r\n1\r\n" ,"STORED\r\n", kNo);
  process(" get a\r\n" ,"VALUE a 0 1\r\n1\r\nEND\r\n", kNo);

  process(" incr a 1\r\n", "2\r\n", kNo);
  process(" decr a 1\r\n", "1\r\n", kNo);
  process(" incr a 2\r\n", "3\r\n", kNo);
  process(" decr a 200\r\n", "0\r\n", kNo);

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

 

void test_fragement()
{
  MemcachedContext context;
  bool closed = false;
  Buffer resp(0, 1024);
  Buffer req(0, 1024);
  context.set_send_func([&resp] (const char* str){ resp.append(str);});
  context.set_close_func([&closed] (){ closed = true;});

  req.append("set a 0 0 3\r\n");

  gProcessor.process(req, context);
  assert(context.get_opt()  == kSet );
  req.append("1\r\n");

  gProcessor.process(req, context);
  assert(context.get_opt()  == kSet );
  assert("" == resp.toString());
  req.append("1\r\n");
  gProcessor.process(req, context);
  assert(context.get_opt()  == kNo);

  cout << resp.toString() << "resp";
  assert("ERROR\r\n" == resp.toString());

};


void test_incr()
{
  MemcachedContext context;
  bool closed = false;
  Buffer resp(0, 1024);
  Buffer req(0, 1024);
  context.set_send_func([&resp] (const char* str){ resp.append(str);});
  context.set_close_func([&closed] (){ closed = true;});

  req.append("set a 0 0 1\r\n1\r\n");
  gProcessor.process(req, context);

  assert("STORED\r\n" == resp.toString());
  resp.clear();

  req.append("incr a ");
  gProcessor.process(req, context);
  cout << "request = " << req.toString() << endl;
  req.append("1\r\n");
  gProcessor.process(req, context);
  cout << "response = " << resp.toString() << endl;
  assert("2\r\n" == resp.toString());

}


int main()
{
  log::LogLevel logLevel = log::LogLevel(0);
  setLogLevel(logLevel);
  test_single_opt();
  test_multi_opt();
  test_fragement();
  test_incr();
  return 0;
}