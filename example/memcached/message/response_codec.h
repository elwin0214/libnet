#ifndef __LIBNET_MEMCACHED_MESSAGE_REQUESTCODEC_H__
#define __LIBNET_MEMCACHED_MESSAGE_REQUESTCODEC_H__
#include <libnet/buffer.h>
#include "message.h"
 
namespace memcached
{
namespace message
{
using namespace libnet;

struct ResponseCodec
{
  void encode(Message& message, Buffer& buffer);
  bool decode(Message& message, Buffer& buffer);

  bool decodeStore(Message& message, Buffer& buffer);
  bool decodeGet(Message& message, Buffer& buffer);
  bool decodeDelete(Message& message, Buffer& buffer);
  bool decodeCount(Message& message, Buffer& buffer);
};

}
}

#endif