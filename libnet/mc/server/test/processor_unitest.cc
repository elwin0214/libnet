#include <string.h>
#include <assert.h>
#include <iostream>
#include <map>
#include <gtest/gtest.h>
#include "../processor.h"

using namespace std;
using namespace libnet;
using namespace mc::server;
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



void process(ProcessorProxy &processor, const char* input, const char* ouput, Opt opt)
{
  MemcachedContext context;
  bool closed = false;
  Buffer resp(0, 1024);
  Buffer req(0, 1024);
  context.set_send_func([&resp] (const char* str){ resp.append(str);});
  context.set_close_func([&closed] (){ closed = true;});
  req.append(input);
  //processor.clear();
  processor.process(req, context);

  ASSERT_EQ(resp.toString(), ouput);
  ASSERT_EQ(opt, context.get_opt());
};

TEST(Processor, single_opt)
{
  ProcessorProxy processor;
  process(processor, " get key1\r\n" ,"END\r\n", kNo);

  process(processor, " set a 0 0 1\r\n1\r\n" ,"STORED\r\n", kNo);
  process(processor, " get a\r\n" ,"VALUE a 0 1\r\n1\r\nEND\r\n", kNo);

  process(processor, " incr a 1\r\n", "2\r\n", kNo);
  process(processor, " decr a 1\r\n", "1\r\n", kNo);
  process(processor, " incr a 2\r\n", "3\r\n", kNo);
  process(processor, " decr a 200\r\n", "0\r\n", kNo);

  process(processor, " replace b 0 0 2\r\n23\r\n" ,"NOT_STORED\r\n", kNo);
  process(processor, " add b 0 0 2\r\n23\r\n" ,"STORED\r\n", kNo);

  process(processor, " delete c\r\n" ,"NOT_FOUND\r\n", kNo);
  process(processor, " delete b\r\n" ,"DELETED\r\n", kNo);
}

TEST(Processor, multi_opt)
{
  ProcessorProxy processor;
  process(processor, " get key1\r\nget b" ,"END\r\n", kNo);
  process(processor, " get key1\r\nget b\r\n" ,"END\r\nEND\r\n", kNo);
  
  process(processor, " add b 0 0 2\r\n23" ,"", kAdd);
  process(processor, " add b 0 0 2\r\n23\r\nget b\r\n" ,"STORED\r\nVALUE b 0 2\r\n23\r\nEND\r\n", kNo);
}

TEST(Processor, fragement)
{
  ProcessorProxy processor;
  MemcachedContext context;
  bool closed = false;
  Buffer resp(0, 1024);
  Buffer req(0, 1024);
  context.set_send_func([&resp] (const char* str){ resp.append(str);});
  context.set_close_func([&closed] (){ closed = true;});

  req.append("set a 0 0 3\r\n");

  processor.process(req, context);
  ASSERT_EQ(context.get_opt(), kSet);
  req.append("1\r\n");

  processor.process(req, context);
  ASSERT_EQ(context.get_opt(), kSet);
  ASSERT_EQ("", resp.toString());
  req.append("1\r\n");
  processor.process(req, context);
  ASSERT_EQ(context.get_opt(), kNo);
  ASSERT_EQ("ERROR\r\n", resp.toString());
};

TEST(Processor, incr)
{
  ProcessorProxy processor;
  MemcachedContext context;
  bool closed = false;
  Buffer resp(0, 1024);
  Buffer req(0, 1024);
  context.set_send_func([&resp] (const char* str){ resp.append(str);});
  context.set_close_func([&closed] (){ closed = true;});

  req.append("set a 0 0 1\r\n1\r\n");
  processor.process(req, context);

  ASSERT_EQ("STORED\r\n", resp.toString());
  resp.clear();

  req.append("incr a ");
  processor.process(req, context);
  req.append("1\r\n");
  processor.process(req, context);
  ASSERT_EQ("2\r\n", resp.toString());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}