#ifndef __LIBNET_MEMCACHED_MESSAGE_RESPONSE_H__
#define __LIBNET_MEMCACHED_MESSAGE_RESPONSE_H__
#include "mcode.h"

namespace memcached
{
namespace message
{

struct Response
{
  Response():code_(kInit), op_(kNo), state_(kLineInit){ }
  Response(Opt op):code_(kInit), op_(op), state_(kLineInit){ }
  Opt op() { return op_; }
  Code code() { return code_; }
  
  Code code_;
  Opt op_;
  std::string key_;
  std::string value_;
  uint32_t count_;
  uint32_t bytes_;
  uint16_t flags_;
  uint32_t exptime_;
  State state_;
};

}
}

#endif
