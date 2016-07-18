#ifndef __LIBNET_HTTP_HTTPCONTEXT_H__
#define __LIBNET_HTTP_HTTPCONTEXT_H__

#include <libnet/http/http_request.h>
#include <libnet/http/http_response.h>
#include <libnet/nocopyable.h>

namespace libnet
{
class Connection;
namespace http
{
namespace state
{
static const char* StateName[13] = 
{
  "kInit",
  "kRequestLineReceived",
  "kRequestLineProcessed",
  "kHeadersReceived",
  "kHeadersProcessed",

  "kBodySending",
  "kBodySent",
  "kPartChunkSending",
  "kPartChunkSizeSending",
  "kPartChunkSizeSent",
  "kPartChunkBodySending",
  "kPartChunkSent",
  "kAllChunkSent"
};

}

class HttpContext : public NoCopyable
{

public:

  enum State
  {
    kInit = 0,
    kRequestLineReceived = 1,
    kRequestLineProcessed = 2,
   // kHeadersPartReceived = 4,
    kHeadersReceived = 3,
    kHeadersProcessed = 4,

    kBodySending = 5,
    kBodySent = 6,


    kPartChunkSending = 7,
    kPartChunkSizeSending = 8,
    kPartChunkSizeSent = 9,
    kPartChunkBodySending = 10,
    kPartChunkSent = 11,
    kAllChunkSent = 12

  };



  HttpContext()
    : state_(kInit),
      request_(),
      response_()
  {

  };

  void reset() { state_ = kInit; request_.reset(); response_.reset(); }
  void setState(State state) { state_ = state; }
  State getState() {return state_; }
  const char* stateToString()
  {
    if ( state_ < kInit || kInit > kAllChunkSent ) return "unknow";
    return state::StateName[state_];
  }

  HttpRequest& getRequest() { return request_; }
  HttpResponse& getResponse() {return response_; }

private:
  State state_;
  HttpRequest request_;
  HttpResponse response_;

};

}
}

#endif


