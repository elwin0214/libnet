#ifndef __LIBNET_MEMCACHED_SERVER_CONTEXT_H__
#define __LIBNET_MEMCACHED_SERVER_CONTEXT_H__
#include <functional>
#include <string>
#include <libnet/nocopyable.h>
#include "../message/request.h"

namespace memcached
{
namespace server
{

class MemcachedContext : public libnet::NoCopyable
{

public:
  MemcachedContext()
  {
  };

  Request& request() { return request_; }
private:
  Request request_;

};


}
}

#endif