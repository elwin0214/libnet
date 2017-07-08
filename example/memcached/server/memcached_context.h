#ifndef __LIBNET_MEMCACHED_SERVER_CONTEXT_H__
#define __LIBNET_MEMCACHED_SERVER_CONTEXT_H__
#include <functional>
#include <string>
#include <libnet/nocopyable.h>
#include "../message/message.h"

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

  Message& request() { return request_; }
private:
  Message request_;

};


}
}

#endif