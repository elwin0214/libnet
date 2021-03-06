#ifndef __LIBNET_MC_MSG_REQUESTCODEC_H__
#define __LIBNET_MC_MSG_REQUESTCODEC_H__
#include <libnet/buffer.h>
#include <libnet/mc/message.h>
 
namespace mc
{
namespace msg
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