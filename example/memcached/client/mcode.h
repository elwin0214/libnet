#ifndef __LIBNET_MEMCACHED_CLIENT_MCODE_H__
#define __LIBNET_MEMCACHED_CLIENT_MCODE_H__

namespace memcached
{
namespace client
{

enum Code
{

  kError, 
  kFail,
  kSucc, 
  kInit
};

enum Opt
{
  kAdd = 0,
  kReplace = 1,
  kSet = 2,
  kGet = 3,
  kDelete = 4,
  kIncr = 5,
  kDecr = 6
};

}
}

#endif
