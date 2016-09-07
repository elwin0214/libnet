#ifndef __LIBNET_MEMCACHED_CLIENT_COMMAND_H__
#define __LIBNET_MEMCACHED_CLIENT_COMMAND_H__

#include <libnet/buffer.h>
#include <libnet/countdown_latch.h>
#include <libnet/logger.h>
#include <string>
#include "code.h"

namespace memcached
{
namespace client
{
using namespace libnet;

template<typename T>
class Command
{
public:
  Command(const char* name, const std::string& key)
    : name_(name),
      key_(key),
      code_(kInit),
      desc_(),
      result_()

  {

  };
  virtual void append(Buffer& buffer) = 0;
  virtual bool parse(Buffer& buffer) = 0;
  Code code(){ return code_; }
  std::string desc() { return desc_; }
  T result() { return result_; }

protected:
  bool isError(Buffer& buffer, const char* crlf)
  {
    bool error = buffer.startWiths("ERROR") || buffer.startWiths("CLIENT_ERROR") || buffer.startWiths("SERVER_ERROR");
    if (error)
    {
      buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
      code_ = kError;
    }
    return error;
  };
protected:
  const char* name_;
  std::string key_;
  Code code_;
  std::string desc_;
  T result_;
};

//req  : set|add|replace <key> <flags> <exptime> <bytes>\r\n<data block>\r\n
//resp : STORED\r\n
//resp : NOT_STORED\r\n
class TextStoreCommand : public Command<bool>
{
public:
  TextStoreCommand(const char* name, const std::string& key, int32_t exptime, const std::string& value)
    : Command(name, key),
      flags_(0),
      exptime_(exptime),
      bytes_((value.size())),
      value_(value)
  {

  };
  virtual void append(Buffer& buffer);
  virtual bool parse(Buffer& buffer);
  virtual std::string getValue() {return value_; }

protected:
  uint16_t flags_;
  uint32_t exptime_;
  uint32_t bytes_;
  std::string value_;
};

//req  : get key\r\n
//resp : VALUE <key> <flags> <bytes> [<cas unique>]\r\n<data block>\r\nEND\r\n  
class GetCommand : public Command<std::string>
{
public:
  GetCommand(const std::string& key)
  : Command("get", key),
    flags_(0),
    bytes_(0),
    getKey_(),
    //value_(),
    state_(kLineInit)
  {

  };

  virtual void append(Buffer& buffer);
  virtual bool parse(Buffer& buffer);
  //virtual std::string getValue() {return value_; }

private:
  bool parseValueLine(Buffer& buffer, const char* crlf);

private:
  uint32_t flags_;
  uint32_t bytes_;
  std::string getKey_;
  //std::string value_;

  enum State
  {
    kLineInit,
    kLineValue,
    kLineData,
    kLineEnd
  };
  State state_;
};
//req  : delete <key>\r\n
//resp : DELETED\r\n
//resp : NOT_FOUND\r\n
class DeleteCommand : public Command<bool>
{
public:
  DeleteCommand(const std::string& key)
    : Command("delete", key)
  {

  }
  virtual void append(Buffer& buffer);
  virtual bool parse(Buffer& buffer);
};
//req : incr|decr <key> <value>\r\n
//resp: <value>\r\n
//resp: NOT_FOUND\r\n
class CountCommand : public Command<uint32_t>
{
public:
  CountCommand(const char* name, const std::string& key, uint32_t value)
    : Command(name, key),
      value_(value)
  {

  }
  virtual void append(Buffer& buffer);
  virtual bool parse(Buffer& buffer);

private:
  uint32_t value_;
};

}
}

#endif