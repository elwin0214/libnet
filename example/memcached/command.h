#ifndef __LIBNET_MEMCACHED_COMMAND_H__
#define __LIBNET_MEMCACHED_COMMAND_H__

#include <libnet/buffer.h>
#include <libnet/countdown_latch.h>
#include <string>

using namespace libnet;

enum Code
{
  kError, 
  kSucc, 
  kNeedMore
};

class Command
{
public:
  Command(const char* name, std::string key)
    : name_(name),
      key_(key),
      code_(kNeedMore)
  {

  };

  virtual void append(Buffer& buffer) = 0;
  virtual void parse(Buffer& buffer) = 0;
  Code code(){ return code_; }
  std::string& result(){return result_; }
  //virtual ~Command() = 0;

protected:
  const char* name_;
  std::string key_;
  std::string result_;
  Code code_;
};

class SetCommand : public Command
{
public:
  SetCommand(std::string key, int32_t exptime, std::string value)
    : Command("set", key),
      flags_(0),
      exptime_(0),
      bytes_((value.size())),
      value_(value)
  {

  };
  virtual void append(Buffer& buffer);
  virtual void parse(Buffer& buffer);
  //virtual ~SetCommand(){ };

protected:
  uint32_t flags_;
  int32_t exptime_;
  int32_t bytes_;
  std::string value_;
};

//get key\r\n
//VALUE <key> <flags> <bytes> [<cas unique>]\r\n<data block>\r\n  
class GetCommand : public Command
{
public:
  GetCommand(std::string key)
  : Command("get", key),
    flags_(0),
    exptime_(0),
    bytes_(0),
    line_(0)
  {

  };

  virtual void append(Buffer& buffer);
  virtual void parse(Buffer& buffer);
  
private:
  uint32_t flags_;
  int32_t bytes_;
  int32_t exptime_;
  int line_;

};

#endif