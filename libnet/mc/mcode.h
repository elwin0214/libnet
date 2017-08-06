#ifndef __LIBNET_MC_MSG_MCODE_H__
#define __LIBNET_MC_MSG_MCODE_H__

namespace mc
{
namespace msg
{

enum Code
{
  kError, 
  kFail,
  kSucc, 
  kInit,
  kConClosed,
  kCacheReject
};

static const char* CodeName[6] = 
{
  "Error",
  "Fail",
  "Succ",
  "Init",
  "ConClosed",
  "CacheReject"
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
  kDecr = 6,
  kVer = 7
};

enum State
{
  kLineInit,
  kLineCmd,  // for rquest
  kLineValue,
  kLineData,
  kLineEnd
};

static const char* OpName[8] = 
{
  "add", 
  "replace", 
  "set", 
  "get",
  "delete",
  "incr",
  "decr",
  "version"
};
static const char* CRLF = "\r\n";

}
}

#endif
