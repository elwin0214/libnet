#ifndef __LIBNET_MEMCACHED_CLIENT_COMMAND_H__
#define __LIBNET_MEMCACHED_CLIENT_COMMAND_H__

#include <libnet/buffer.h>
#include <libnet/countdown_latch.h>
#include <string>
#include "../message/mcode.h"
#include "../message/message.h"
#include "../message/request_codec.h"
#include "../message/response_codec.h"

namespace memcached
{
namespace client
{
using namespace libnet;
using namespace std;
using namespace memcached::message;
class Command
{
public:
  typedef std::function<void()> Callback;
  Command(Message request)
    : request_(request),
      response_(request.op()),
      reqc_(),
      respc_(),
      callback_()
  {
  }

  Command(Message request, Callback callback)
    : request_(request),
      response_(request.op()),
      reqc_(),
      respc_(),
      callback_(callback)
  {
  }

  void encode(Buffer& buffer) 
  { 
    reqc_.encode(request_, buffer);
  }

  bool decode(Buffer& buffer) 
  {
    bool r = respc_.decode(response_, buffer);
    if (r && callback_)
      callback_();
    return r;
  }

  void call()
  {
    if (callback_) callback_();
  }

  Message& response() { return response_; }

  
protected:
  Message request_;
  Message response_;
  RequestCodec reqc_;
  ResponseCodec respc_;
  Callback callback_;
  //std::shared_ptr<CountDownLatch> latch_;
};

//RequestCodec Command::reqc_ = RequestCodec();
//ResponseCodec Command::respc_ = ResponseCodec();

}
}

#endif