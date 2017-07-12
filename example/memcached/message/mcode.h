#ifndef __LIBNET_MEMCACHED_MESSAGE_MCODE_H__
#define __LIBNET_MEMCACHED_MESSAGE_MCODE_H__

namespace memcached
{
namespace message
{

enum Code
{
  kError, 
  kFail,
  kSucc, 
  kInit,

  kConClosed
};

enum Opt
{
  kNo = -1,
  kAdd = 0,
  kReplace = 1,
  kSet = 2,
  kGet = 3,
  kDelete = 4,
  kIncr = 5,
  kDecr = 6
};

enum State
{
  kLineInit,
  kLineCmd,  // for rquest
  kLineValue,
  kLineData,
  kLineEnd
};

static const char* OpName[7] = 
{
  "add", 
  "replace", 
  "set", 
  "get",
  "delete",
  "incr",
  "decr"
};
static const char* CRLF = "\r\n";

}
}

#endif
