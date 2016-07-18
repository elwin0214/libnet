#include <map>
#include <string>
#include <libnet/connection.h>
#include <libnet/http/http_response.h>
#include <libnet/digits.h>
#include <libnet/logger.h>

#define BLOCK(m) { (sizeof(m) / sizeof(char*)), m }

namespace libnet
{
namespace http
{
static const char *invalid_hundred[] = { NULL };

static const char *const one_hundred[] = {
  "Continue",
  "Switching Protocols",
  "Processing"
};

static const char *const two_hundred[] = {
  "OK",
  "Created",
  "Accepted",
  "Non-Authoritative Information",
  "No Content",
  "Reset Content",
  "Partial Content",
  "Multi Status"
};

static const char *const three_hundred[] = {
  "Multiple Choices",
  "Moved Permanently",
  "Moved Temporarily",
  "See Other",
  "Not Modified",
  "Use Proxy",
  "Switch Proxy",
  "Temporary Redirect"
};

static const char *const four_hundred[] = {
  "Bad Request",
  "Unauthorized",
  "Payment Required",
  "Forbidden",
  "Not Found",
  "Method Not Allowed",
  "Not Acceptable",
  "Proxy Authentication Required",
  "Request Time-out",
  "Conflict",
  "Gone",
  "Length Required",
  "Precondition Failed",
  "Request Entity Too Large",
  "Request-URI Too Large",
  "Unsupported Media Type",
  "Requested Range Not Satisfiable",
  "Expectation Failed",
  "Unknown",
  "Unknown",
  "Unknown", /* 420 */
  "Unknown",
  "Unprocessable Entity",
  "Locked",
  "Failed Dependency",
  "Unordered Collection",
  "Upgrade Required",
  "Unknown",
  "Unknown",
  "Unknown",
  "Unknown", /* 430 */
  "Unknown",
  "Unknown",
  "Unknown",
  "Unknown",
  "Unknown", /* 435 */
  "Unknown",
  "Unknown",
  "Unknown",
  "Unknown",
  "Unknown", /* 440 */
  "Unknown",
  "Unknown",
  "Unknown",
  "No Response",
  "Unknown", /* 445 */
  "Unknown",
  "Unknown",
  "Unknown",
  "Retry With",
  "Blocked by Windows Parental Controls", /* 450 */
  "Unavailable For Legal Reasons"
};

static const char *const five_hundred[] = {
  "Internal Server Error",
  "Not Implemented",
  "Bad Gateway",
  "Service Unavailable",
  "Gateway Time-out",
  "HTTP Version not supported",
  "Variant Also Negotiates",
  "Insufficient Storage",
  "Unknown",
  "Bandwidth Limit Exceeded",
  "Not Extended"
};

struct Reason
{
  unsigned int max;
  const char *const*data;
};

static const struct Reason reasons[] = {
  BLOCK (invalid_hundred),
  BLOCK (one_hundred),
  BLOCK (two_hundred),
  BLOCK (three_hundred),
  BLOCK (four_hundred),
  BLOCK (five_hundred),
};

static const char * getReason(size_t code)
{
  if ( (code >= 100) &&
       (code < 600) &&
       (reasons[code / 100].max > (code % 100)) )
    return reasons[code / 100].data[code % 100];
  return "Unknown";
};

void HttpResponse::appendToBuffer(Buffer *output)
{
  char buf[32];
  snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_);
  output->append(buf);
  output->append(getReason(status_));
  output->append("\r\n");

  LOG_INFO << "chucked = " << chunked_ ;
  if (close_)
  {
    output->append("Connection: close\r\n");
  }
  else
  {
    if (!chunked_)
    {
      snprintf(buf, sizeof(buf), "Content-Length: %zu\r\n", buffer_->readable());
      output->append(buf);
    }
    else
    {
      output->append("Transfer-Encoding: chunked\r\n");
    }
    output->append("Connection: Keep-Alive\r\n");
  }
  for (Headers::iterator itr = headers_.begin(); itr != headers_.end(); itr++)
  {
    output->append(itr->first);
    output->append(": ");
    output->append(itr->second);
    output->append("\r\n");
  }
  output->append("\r\n");

};

void HttpResponse::send(const char* str, size_t len)
{ 
  assert(!sending_ || chunked_);
  if (sending_ && !chunked_)
  {
    LOG_WARN << "not chunked encoding!" ;
    return;
  }
  buffer_->append(str, len);
};

void HttpResponse::flush()
{
  if(!sending_) //发送 headers
  {
    sending_ = true;
    //std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(512);
    Buffer* buffer = new Buffer(kPrepend, kSize);
    appendToBuffer(buffer);
    sendCallback_(buffer);
  }
  if (!chunked_)
  {
    if (buffer_->readable() > 0)
    {
      sendCallback_(buffer_.release());
      buffer_.reset(new Buffer(kPrepend, kSize));
    }
  }
  else
  {
    size_t len = buffer_->readable();
    char buf[32];
    int n = digits::digitToXstring(len, buf);
    assert(n > 0);
    assert(n + 2 <= 32);
    buf[n] = '\r';
    buf[n + 1] = '\n';
    //LOG_INFO << "n = " << n <<" size = " << std::string(buf, n + 2) ;
    //LOG_INFO << "buffer = " << (buffer_->toString());
    buffer_->prepare(buf, n + 2);
    //LOG_INFO << "buffer = " << (buffer_->toString());
    buffer_->append("\r\n");
    sendCallback_(buffer_.release());
    buffer_.reset(new Buffer(kPrepend, kSize));
  }
};

void HttpResponse::finish()
{
  if (chunked_)
  {
    send("0\r\n\r\n");
  }
};



}
}