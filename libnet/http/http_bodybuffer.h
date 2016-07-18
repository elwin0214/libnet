#ifndef __LIBNET_HTTP_HTTPBODY_H__
#define __LIBNET_HTTP_HTTPBODY_H__

#include <libnet/http/http_request.h>

namespace libnet
{
namespace http
{

struct BodyBuffer
{
  BodyBuffer():chunked_(false),size_(0),offset_(0) { }

  bool chunked_;
  size_t size_;
  size_t offset_; //processed

  void swap(BodyBuffer &buffer) {
    std::swap(chunked_, buffer.chunked_);
    std::swap(size_, buffer.size_);
    std::swap(offset_, buffer.offset_);
  }
  bool isEnd() { return size_ == offset_; }
  size_t remain() { return size_ - offset_; }
  void reset() {chunked_ = false; size_ = 0; offset_ = 0; }

};

}
}
#endif