#ifndef __LIBNET_HTTP_HTTPRESPONSE_H__
#define __LIBNET_HTTP_HTTPRESPONSE_H__

#include <memory>
#include <map>
#include <string>
#include <string.h>
#include <libnet/nocopyable.h>
#include <libnet/buffer.h>
#include <libnet/connection.h>

namespace libnet
{
class Buffer;
namespace http
{
class HttpContext;

class HttpResponse : public NoCopyable
{
public:
  typedef std::function<void(Buffer*)> SendCallback;
  typedef std::function<void(CString)> SendStringCallback;
  typedef std::map<std::string, std::string> Headers ;
public:
  enum StatusCode
  {
    OK = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    
    BadRequest = 400,
    NotFound = 404
  };
  static const size_t kPrepend = 16;
  static const size_t kSize = 512;

  HttpResponse()
    : status_(0),
      close_(false),
      sending_(false),
      chunked_(false),
      headers_(),
      sendCallback_(),
      buffer_(new Buffer(kPrepend, kSize))
  {

  }

   
  void setClose(bool close) { close_ = close; }
  bool isClose() { return close_; }
  void setStatus(StatusCode status) { status_ = status; }
  void addHeader(const std::string &name, const std::string &value) { headers_.insert(std::pair<std::string,std::string>(name, value)); }

  void reset()
  {
    status_ = 0; 
    close_ = false;
    sending_ = false;
    chunked_ = false;
    Headers headers;
    headers_.swap(headers);
    buffer_.reset(new Buffer());
  }

  void setChunked(bool chunked) {chunked_ = chunked; }
  bool isChunked() {return chunked_; }
  void send(const char* str, size_t len);

  void flush();
  void setSendCallback(SendCallback sendCallback) { sendCallback_ = sendCallback; }
  void setSendStringCallback(SendStringCallback sendCallback) { sendStringCallback_ = sendCallback; }

  void finish();
private:
  void appendToBuffer(Buffer *buffer);
  void send(const char* str) { sendStringCallback_(CString(str, ::strlen(str)));}
  void send(Buffer* buffer) { sendCallback_(buffer); }

  int status_;
  bool close_;
  bool sending_;
  bool chunked_;
  Headers headers_;
  SendCallback sendCallback_;
  SendStringCallback sendStringCallback_;
  std::unique_ptr<Buffer> buffer_;
  
};

}
}

#endif
