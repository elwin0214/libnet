#ifndef __LIBNET_MEMCACHED_MESSAGE_REQUEST_H__
#define __LIBNET_MEMCACHED_MESSAGE_REQUEST_H__
#include <libnet/buffer.h>
#include <string>
#include <assert.h>
#include "mcode.h"

namespace memcached
{
namespace message
{
using namespace std;
using namespace libnet;
struct Request
{
  Request()
    : code_(kInit),
      op_(kNo), 
      count_(0),
      flags_(0),
      exptime_(0),
      bytes_(0),
      state_(kLineInit)
  {

  }
  
  Request(Opt op, std::string key)
    : code_(kInit),
      op_(op),
      key_(key),
      count_(0),
      flags_(0),
      exptime_(0),
      bytes_(0),
      state_(kLineInit)
  {

  }

  Request(Opt op, std::string key, uint32_t count)
    : code_(kInit),
      op_(op),
      key_(key),
      count_(count),
      flags_(0),
      exptime_(0),
      bytes_(0),
      state_(kLineInit)
  {

  }

  Request(Opt op, std::string key, std::string value, uint16_t flags, uint32_t exptime)
    : code_(kInit),
      op_(op),
      key_(key),
      value_(value),
      count_(0),
      flags_(flags),
      exptime_(exptime),
      state_(kLineInit)
  {

  }

  bool operator==(Request& req)
  {
    return this == &req;
    return (op_ == req.op_) 
           && (code_ == req.code_)
           && (key_ == req.key_)
           && (value_ == req.value_)
           && (count_ == req.count_)
           && (flags_ == req.flags_)
           && (exptime_ == req.exptime_)
           && (bytes_ == req.exptime_)
           && (state_ == req.state_);

  }

  Opt op() { return op_; }
  Code code() { return code_; }

  const char* op_name(){ if (op_ > 0) return " "; else return OpName[op_]; }

  void reset()
  {

    code_ = kInit;
    op_ = kNo;
    string key;
    key_.swap(key);
    string value;
    value_.swap(value);
    count_ = 0;
    flags_ = 0;
    exptime_ = 0;
    bytes_ = 0;
    state_ = kLineInit;
  }

  Code code_;
  Opt op_;
  std::string key_;
  std::string value_;
  uint32_t count_;
  uint16_t flags_;
  uint32_t exptime_;
  uint32_t bytes_;
  State state_;
};

}
}

#endif