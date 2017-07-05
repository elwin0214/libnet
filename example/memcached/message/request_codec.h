#ifndef __LIBNET_MEMCACHED_MESSAGE_RESPONSECODEC_H__
#define __LIBNET_MEMCACHED_MESSAGE_RESPONSECODEC_H__
#include <libnet/buffer.h>
#include "request.h"
#include "response.h"

namespace memcached
{
namespace message
{
using namespace libnet;

struct RequestCodec
{
  void encode(Request& request, Buffer& buffer);
  bool decode(Request& request, Buffer& buffer);

  bool decodeSimple(Request& request, Buffer& buffer);
  bool decodeStore(Request& request, Buffer& buffer);
  bool decodeCount(Request& request, Buffer& buffer);

};

}
}

#endif
