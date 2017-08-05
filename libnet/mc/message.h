#ifndef __LIBNET_MC_MSG_REQUEST_H__
#define __LIBNET_MC_MSG_REQUEST_H__
#include <libnet/buffer.h>
#include <string>
#include <assert.h>
#include "mcode.h"

namespace mc
{
namespace msg
{
using namespace std;
using namespace libnet;
struct Data
{
  Data()
    : op_(kNo), 
      count_(0),
      flags_(0),
      exptime_(0),
      bytes_(0)
  {

  }
  
  Data(Opt op)
    : op_(op), 
      count_(0),
      flags_(0),
      exptime_(0),
      bytes_(0)
  {

  }

  Data(Opt op, std::string key)
    : op_(op),
      key_(key),
      count_(0),
      flags_(0),
      exptime_(0),
      bytes_(0)
  {

  }

  Data(Opt op, std::string key, uint32_t count)
    : op_(op),
      key_(key),
      count_(count),
      flags_(0),
      exptime_(0),
      bytes_(0)
  {

  }

  Data(Opt op, std::string key, std::string value, uint16_t flags, uint32_t exptime)
    : op_(op),
      key_(key),
      value_(value),
      count_(0),
      flags_(flags),
      exptime_(exptime)
  {

  }
  Opt op() { return op_; }
  const char* op_name(){ if (op_ < 0) return " "; else return OpName[op_]; }

  void reset()
  {
    op_ = kNo;
    string key;
    key_.swap(key);
    string value;
    value_.swap(value);
    count_ = 0;
    flags_ = 0;
    exptime_ = 0;
    bytes_ = 0;
  }

  Opt op_;
  std::string key_;
  std::string value_;
  uint32_t count_;
  uint16_t flags_;
  uint32_t exptime_;
  uint32_t bytes_;

};

struct Stat
{
  Stat():code_(kInit),line_(kLineInit) {}
  void reset() { code_ = kInit, line_ = kLineInit; }
  Code code_;
  State line_;
};

struct Message
{
  Stat stat_;
  Data data_;
  Message():stat_(), data_() {}
  Message(Opt op):stat_(), data_(op) {}
  Message(Opt op, std::string key):stat_(), data_(op, key) {}
  Message(Opt op, std::string key, uint32_t count): stat_(), data_(op, key, count) {}
  Message(Opt op, std::string key, std::string value, uint16_t flags, uint32_t exptime)
    : stat_(),
      data_(op, key, value, flags, exptime)
  {

  }
  Opt op() { return data_.op(); }
  const char* op_name() { return data_.op_name(); }
  Code code() { return stat_.code_; }
  const char* code_name() { if (stat_.code_ < 0) return " "; else return CodeName[stat_.code_]; }
  void reset() { stat_.reset(); data_.reset(); }
};

}
}

#endif