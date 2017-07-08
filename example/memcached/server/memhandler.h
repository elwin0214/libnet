#ifndef __LIBNET_MEMCACHED_SERVER_HANDLER_H__
#define __LIBNET_MEMCACHED_SERVER_HANDLER_H__
#include <libnet/nocopyable.h>
#include "memcache.h"
#include "../message/message.h"

namespace memcached
{
namespace server
{
using namespace memcached::message;

class MemHandler : public NoCopyable
{

public:
  MemHandler()
    : cache_()
  {

  }

  MemHandler(size_t hashpower, double factor, const SlabOption& slab_options)
    : cache_(hashpower, factor, slab_options)
  {

  }

  void handle(Message& request, Message& response);

private:
  MemCache cache_;

};

}
}

#endif