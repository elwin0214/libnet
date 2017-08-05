#ifndef __LIBNET_MC_MSG_RESPONSECODEC_H__
#define __LIBNET_MC_MSG_RESPONSECODEC_H__
#include <libnet/buffer.h>
#include <libnet/mc/message.h>

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

}
}

#endif
