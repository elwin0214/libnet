#ifndef __LIBNET_MC_MSG_RESPONSECODEC_H__
#define __LIBNET_MC_MSG_RESPONSECODEC_H__
#include <libnet/buffer.h>
#include "message.h"

namespace mc
{
namespace msg
{
using namespace libnet;

struct RequestCodec
{
  void encode(Message& message, Buffer& buffer);
  bool decode(Message& message, Buffer& buffer);

  bool decodeSimple(Message& message, Buffer& buffer);
  bool decodeStore(Message& message, Buffer& buffer);
  bool decodeCount(Message& message, Buffer& buffer);
};

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
