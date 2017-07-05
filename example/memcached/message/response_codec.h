#ifndef __LIBNET_MEMCACHED_MESSAGE_REQUESTCODEC_H__
#define __LIBNET_MEMCACHED_MESSAGE_REQUESTCODEC_H__
#include <libnet/buffer.h>
#include "request.h"
#include "response.h"
 
namespace memcached
{
namespace message
{
using namespace libnet;

struct ResponseCodec
{
  void encode(Response& response, Buffer& buffer);
  bool decode(Response& response, Buffer& buffer);

  bool decodeStore(Response& response, Buffer& buffer);
  bool decodeGet(Response& response, Buffer& buffer);
  bool decodeDelete(Response& response, Buffer& buffer);
  bool decodeCount(Response& response, Buffer& buffer);
};

}
}

#endif