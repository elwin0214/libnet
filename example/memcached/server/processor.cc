#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <libnet/digits.h>
#include <libnet/logger.h>
#include "processor.h"
#include "item.h"

namespace memcached
{
namespace server
{
using namespace libnet::digits;
using namespace std::placeholders;
static const size_t kMaxKeySize = 255;
static const size_t kMaxValueSize = 65535;

class GetProcessor : public Processor
{
public:
  GetProcessor(Opt opt)
    : Processor(opt)
  {

  }
  virtual bool process(Buffer& buffer, MemcachedContext& context);
};


class CounterProcessor : public Processor
{
public:
  CounterProcessor(Opt opt)
    : Processor(opt)
  {

  }

  virtual bool process(Buffer& buffer, MemcachedContext& context);

};

class DeleteProcessor : public Processor
{
public:
  DeleteProcessor(Opt opt)
    : Processor(opt)
  {

  }

  virtual bool process(Buffer& buffer, MemcachedContext& context);
 
};

class TextStoreProcessor : public Processor
{
public:
  TextStoreProcessor(Opt opt)
    : Processor(opt)
  {

  }

  virtual bool process(Buffer& buffer, MemcachedContext& context);
};


bool Processor::isDigit(const char* str)
{
  while(!(*str != '\0'))
  {
    if (!::isdigit(*str)) return false;
  } 
  return true;
};

bool GetProcessor::process(Buffer& buffer, MemcachedContext& context)
{
  const char* end = buffer.find("\r\n");
  assert(NULL != end);
  const char* start = buffer.beginRead();
  Tokenizer tokenizer(start, end, ' ');

  const char* pos = NULL;
  size_t len = -1;
  bool next = tokenizer.next(pos, len);
  if (!next)
  {
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.send("ERROR\r\n");
    return true;
  }

  std::string key = std::string(pos, len);
  buffer.moveReadIndex(end + 2 - buffer.beginRead());
  if (key.size() >= kMaxKeySize)
  {
    context.send("ERROR\r\n");
    return true;
  }
  Item* item = item_find_func_(key.c_str());
  if (NULL == item)
  {
    context.send("END\r\n");
    return true;
  }

  context.send("VALUE ");
  context.send(item->key());

  char buf[64]; 
  ::bzero(buf, sizeof(buf));
  ::sprintf(buf, " %d %d\r\n", item->get_flags(), item->get_bytes());
  //std::to_string(item->get_flags())
  context.send(buf);
  context.send(item->value());
  context.send("\r\nEND\r\n");
  return true;
};

bool CounterProcessor::process(Buffer& buffer, MemcachedContext& context)
{
  const char* end = buffer.find("\r\n");
  assert(NULL != end);
  const char* start = buffer.beginRead();
  Tokenizer tokenizer(start, end, ' ');
  const char* pos = NULL;
  size_t len = -1;
  bool next = tokenizer.next(pos, len);
  if (!next)
  {
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.send("ERROR\r\n");
    return true;
  }

  std::string key = std::string(pos, len);
  if (key.size() >= kMaxKeySize)
  {
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.send("ERROR\r\n");
    return true;
  }

  Item* item = item_find_func_(key.c_str());
  if (NULL == item)
  {
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.send("NOT_FOUND\r\n");
    return true;
  }

  if (!isDigit(item->value()))
  {
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.send("CLIENT_ERROR\r\n");
    return true;
  }

  uint32_t value;
  try 
  {
    value = std::stoi(item->value(), nullptr, 10);
  }
  catch(...)
  { 
    LOG_ERROR << "key = " << key << " error = convert value to int fail!"; 
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.send("ERROR\r\n");
    return true;
  }
  buffer.moveReadIndex(end + 2 - buffer.beginRead());
  if (opt_ == kIncr)
    value++;
  else
    value--;
  //if (value < 0) value = 0;
  
  std::string value_str = std::to_string(value);
  item->set_value(value_str.c_str(), value_str.size()); // should keep the min item can save a uint32_t number
  context.send(item->value());
  context.send("\r\n");
  
  return true;
};

bool DeleteProcessor::process(Buffer& buffer, MemcachedContext& context)
{
  const char* end = buffer.find("\r\n");
  assert(NULL != end);
  const char* start = buffer.beginRead();
  Tokenizer tokenizer(start, end, ' ');

  const char* pos = NULL;
  size_t len = -1;
  bool next = tokenizer.next(pos, len);
  if (!next)
  {
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.send("ERROR\r\n");
    return true;
  }

  std::string key = std::string(pos, len);
  buffer.moveReadIndex(end + 2 - buffer.beginRead());

  if (key.size() >= kMaxKeySize)
  {
    context.send("ERROR\r\n");
    return true;
  }

  Item* item = item_find_func_(key.c_str());
  if (NULL == item)
  {
    context.send("NOT_FOUND\r\n");
    return true;
  }
  item_remove_func_(item);
  context.send("DELETED\r\n");
  return true;
};
// kAdd kReplace kSet
//<command name> <key> <flags> <exptime> <bytes>"r"n

bool TextStoreProcessor::process(Buffer& buffer, MemcachedContext& context)
{
  if (context.get_key().size() == 0) //did not get header 
  {
    const char* end = buffer.find("\r\n");
    assert(NULL != end);
    const char* start = buffer.beginRead();
    Tokenizer tokenizer(start, end, ' ');

    const char* pos = NULL;
    size_t len = -1;
    bool next = tokenizer.next(pos, len);
    //key
    if (!next)
    {
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }

    std::string key = std::string(pos, len);
    if (key.size() >= kMaxKeySize)
    {
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }

    buffer.moveReadIndex(pos + len - buffer.beginRead());
    //flags
    if (!tokenizer.next(pos, len))
    {
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }

    std::string flags = std::string(pos, len);
    buffer.moveReadIndex(pos + len - buffer.beginRead());
    int flags_int = 0;
    try
    {
      flags_int = std::stoi(flags.c_str(), nullptr, 10);  
    }
    catch(...)
    { 
      LOG_ERROR << "key = " << key << " error = convert " << flags << " to int fail!"; 
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }
    if (flags_int < 0 || flags_int > std::numeric_limits<uint16_t>::max())
    {
      LOG_ERROR << "flags = " << flags << " error = overflow!"; 
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
    }
    //exptime
    if (!tokenizer.next(pos, len))
    {
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }
    std::string exptime = std::string(pos, len);
    buffer.moveReadIndex(pos + len - buffer.beginRead());
    uint32_t exptime_int = 0;
    
    try
    {
      exptime_int = std::stoi(exptime.c_str(), nullptr, 10);
    }
    catch(...)
    { 
      LOG_ERROR << "exptime = " << exptime << " error = convert to int fail!"; 
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }

    //todo

    //bytes
    if (!tokenizer.next(pos, len))
    {
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }
    std::string bytes = std::string(pos, len);
    buffer.moveReadIndex(pos + len - buffer.beginRead());
    uint32_t bytes_int = 0;
    //digits::stringToDigit(bytes.c_str(), &bytes_int);
    try
    {
      bytes_int = std::stoi(bytes.c_str(), nullptr, 10);
    }
    catch(...)
    { 
      LOG_ERROR << "bytes = " << bytes << " error = convert to int fail!"; 
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }
    if (bytes_int > kMaxValueSize)
    {
      LOG_ERROR << "key = " << key << " bytes = " << bytes_int << " greater than max value!"; 
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    context.set_key(std::move(key));
    context.set_flags(flags_int);
    context.set_exptime(exptime_int);
    context.set_bytes(bytes_int);
    return false;
  }
  else
  {//process body
    if (buffer.readable() < context.get_bytes() + 2)
    {
      LOG_TRACE << "need " << (context.get_bytes()) << " bytes for the value!";
      return false;
    }
    LOG_INFO << buffer.toString() << "process";

    const char* end = buffer.find(context.get_bytes(), "\r\n");
    if (NULL == end)
    {
      LOG_TRACE << "can not find crlf" ;
      return false;// can not find \r\n,but the length is over
    }
    if (context.get_bytes() != end - buffer.beginRead())
    {
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }
    std::string& key = context.get_key();
    Item* old_item = item_find_func_(key.c_str());
    if (NULL == old_item)
    {
      if (opt_ == kReplace)
      {
        buffer.moveReadIndex(end + 2 - buffer.beginRead());
        context.send("NOT_STORED\r\n");
        return true;
      }
    }
    else
    {
      if (opt_ == kAdd)
      {
        buffer.moveReadIndex(end + 2 - buffer.beginRead());
        context.send("NOT_STORED\r\n");
        return true;
      }
      else
      {
        item_remove_func_(old_item);
      }
    }
    size_t item_size = key.size() + context.get_bytes() + 2;
    Item* item = item_alloc_func_(item_size);
    if (NULL == item)
    {
      LOG_ERROR << "can not alloc ";
      buffer.moveReadIndex(end + 2 - buffer.beginRead());
      context.send("ERROR\r\n");
      return true;
    }
    item->set_flags(context.get_flags());
    item->set_exptime(context.get_exptime());
    
    item->set_key(key.c_str(), key.size());
    item->set_value(buffer.beginRead(), context.get_bytes());
    item_add_func_(item); // not existed item
    context.send("STORED\r\n");
    buffer.moveReadIndex(end + 2 - buffer.beginRead());
    return true;
  }
};


MemcachedProcessor::MemcachedProcessor()
{

};

void MemcachedProcessor::init()
{
  processors_.reserve(7);
  processors_.emplace_back(new TextStoreProcessor(kAdd));
  processors_.emplace_back(new TextStoreProcessor(kReplace));
  processors_.emplace_back(new TextStoreProcessor(kSet));
  processors_.emplace_back(new GetProcessor(kGet));
  processors_.emplace_back(new DeleteProcessor(kDelete));
  processors_.emplace_back(new CounterProcessor(kIncr));
  processors_.emplace_back(new CounterProcessor(kDecr));

  for (auto processor : processors_)
  {
    processor->item_find_func_ = item_find_func_;
    processor->item_remove_func_ = item_remove_func_;
    processor->item_alloc_func_ = item_alloc_func_;
    processor->item_add_func_ = item_add_func_;
  }
};

void MemcachedProcessor::process(Buffer& buffer, MemcachedContext& context)
{ 
  while (true)
  {
    Opt opt = context.get_opt();
    if (opt == kNo)
    {
      const char* end = buffer.find("\r\n"); 
      if (NULL == end)
      {
        if (buffer.readable() > kMaxValueSize + 2)
        {
          LOG_ERROR << "buffer line too long, so to close!";
          context.close();//todo
        }
        return;
      } 
      Tokenizer tokenizer(buffer.beginRead(), end, ' ');
      const char* pos = NULL;
      size_t len = 0;
      bool next = tokenizer.next(pos, len);
      if (!next)
      {
        buffer.moveReadIndex(end + 2 - buffer.beginRead());
        context.send("ERROR\r\n");
        continue;
      }
      assert(NULL != pos);
      buffer.moveReadIndex(pos + len - buffer.beginRead());

      std::string cmd = std::string(pos, len);
      LOG_DEBUG << "cmd = " << cmd ;
      Opt opt = kNo;
      if (cmd == "get")
        opt = kGet;
      else if (cmd == "set")
        opt = kSet;
      else if (cmd == "add")
        opt = kAdd;
      else if (cmd == "replace")
        opt = kReplace;
      else if (cmd == "delete")
        opt = kDelete;
      else if (cmd == "incr")
        opt = kIncr;
      else if (cmd == "decr")
        opt = kDecr;
      else if (cmd == "quit")
      {
        //LOG_INFO <<  ;
        context.close();
        return;
      }
      else
      {
        buffer.moveReadIndex(end + 2 - buffer.beginRead());
        context.send("ERROR\r\n");
        continue;
      }
      context.set_opt(opt);
      if (processors_[opt]->process(buffer, context)) //processed a request
      {
        context.reset();
      }
    }
    else if(opt == kAdd || opt ==kReplace || opt == kSet)
    {
      if (buffer.readable() < context.get_bytes() + 2)
        return;// need more
      const char* end = buffer.find(context.get_bytes(), "\r\n");
      if (end == NULL)
      {
        if (buffer.readable() > kMaxValueSize + 2)// didn't get body and buffer is too long 
        {
          LOG_ERROR << "buffer line too long, so to close!";
          context.close();
        }
        return;
      }
      if (processors_[opt]->process(buffer, context)) //process body
      {
        context.reset();
      }
      else
      {
      }
    }
    else
    {
      LOG_SYSFATAL << "should not run here!" ;
      assert(false);
    }
  }
};

}
}

