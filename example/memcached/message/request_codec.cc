#include <assert.h>
#include <libnet/digits.h>
#include <libnet/logger.h>
#include "request_codec.h"
#include "reader.h"
#include "mcode.h"

namespace memcached
{
namespace message
{
using namespace libnet;
using namespace std;
static const size_t kMaxKeySize = 255;
static const size_t kMaxValueSize = 65535;
void RequestCodec::encode(Request& request, Buffer& buffer)
{
  if (request.op() == Opt::kSet || request.op() == Opt::kAdd || request.op() == Opt::kReplace)
  {
    buffer.append(OpName[request.op_]);
    buffer.append(" ");
    buffer.append(request.key_);
    char buf[128];
    ::bzero(buf, sizeof(buf));
    snprintf(buf, sizeof(buf), " %d %d %lu", request.flags_, request.exptime_, request.value_.size());
    buffer.append(buf);
    buffer.append(CRLF);
    buffer.append(request.value_);
    buffer.append(CRLF);
  }
  else if(request.op() == Opt::kGet)
  { 
    buffer.append(OpName[Opt::kGet]);
    buffer.append(" ");
    buffer.append(request.key_);
    buffer.append(CRLF);
  }
  else if(request.op() == Opt::kDelete)
  {
    buffer.append(OpName[Opt::kDelete]);
    buffer.append(" ");
    buffer.append(request.key_);
    buffer.append(CRLF);
  }
  else if(request.op() == Opt::kIncr || request.op() == Opt::kDecr)
  {
    buffer.append(OpName[request.op()]);
    buffer.append(" ");
    buffer.append(request.key_);
    buffer.append(" ");
    char buf[16];
    int n = sprintf(buf, "%d", request.count_);
    assert(n > 0);
    buffer.append(buf, n);
    buffer.append(CRLF);
  }
  else
  {
    assert(false);
  }
};


bool RequestCodec::decode(Request& request, Buffer& buffer)
{ 
  
  if (request.op_ == kNo)
  {
    const char* end = buffer.find(CRLF);
    if (NULL == end) return false; 
    buffer.trim();
    if (buffer.startWiths("get")) request.op_ = kGet;
    else if (buffer.startWiths("set")) { request.op_ = kSet; request.state_ = kLineCmd; }
    else if (buffer.startWiths("replace")) { request.op_ = kReplace; request.state_ = kLineCmd; }
    else if (buffer.startWiths("add")) { request.op_ = kAdd; request.state_ = kLineCmd; }
    else if (buffer.startWiths("delete")) request.op_ = kDelete;
    else if (buffer.startWiths("incr")) request.op_ = kIncr;
    else if (buffer.startWiths("decr")) request.op_ = kDecr;
    else
    {
      buffer.skip(end - buffer.beginRead() + 2);
      request.code_ = kError;
      return true;
    }
    LOG_TRACE << "op = " << request.op_ ;
  }

  if (request.op_ == kGet) return decodeSimple(request, buffer);
  if (request.op_ == kSet) return decodeStore(request, buffer);
  if (request.op_ == kReplace) return decodeStore(request, buffer);
  if (request.op_ == kAdd) return decodeStore(request, buffer);
  if (request.op_ == kDelete) return decodeSimple(request, buffer);
  if (request.op_ == kIncr || request.op_ == kDecr) return decodeCount(request, buffer);
  else
  {
    LOG_ERROR << "unknow op!"; 
    return true;
  }
};

bool RequestCodec::decodeSimple(Request& request, Buffer& buffer)
{
  const char* end = buffer.find(CRLF);
  if (NULL == end) return false;
  const char* start = buffer.beginRead();
  Reader reader(start, end, ' ');
  string word;
  if (!(reader.read(word)))request.code_ = kError;
  if (!reader.read(word)) request.code_ = kError;

  if (request.code_ != kError)
  {
    request.code_ = kSucc;
    request.key_ = word;
  }
  buffer.skip(end - start + 2);
  return true;  
};

bool RequestCodec::decodeCount(Request& request, Buffer& buffer)
{
  const char* end = buffer.find(CRLF);
  if (NULL == end) return false;
  const char* start = buffer.beginRead();
  Reader reader(start, end, ' ');
  string word;
  uint32_t count;
  if (!(reader.read(word)))request.code_ = kError;
  if (!reader.read(word)) request.code_ = kError;
  if (!(reader.read(word) && digits::convert<uint32_t>(word.c_str(), count, 10)))request.code_ = kError;

  if (request.code_ != kError)
  {
    request.code_ = kSucc;
    request.key_ = word;
    request.count_ = count;
  }
  buffer.skip(end - start + 2);
  return true; 
};

bool RequestCodec::decodeStore(Request& request, Buffer& buffer)
{
  while (request.code_ == kInit)
  {
    switch(request.state_)
    {
      case kLineCmd:
      {
        const char* end = buffer.find(CRLF);
        if (NULL == end) return false;
        const char* start = buffer.beginRead();
        Reader reader(start, end, ' ');
        uint16_t flags;
        uint32_t bytes;
        uint32_t exptime;
        string key;
        string word;
        if (!reader.read(word)) request.code_ = kError;
        if (!(reader.read(key) && key.size() <= kMaxKeySize)) request.code_ = kError;
        if (!(reader.read(word) && word.size() >0 && digits::convert<uint16_t>(word.c_str(), flags, 10))) request.code_ = kError;
        if (!(reader.read(word) && word.size() >0 && digits::convert<uint32_t>(word.c_str(), exptime, 10))) request.code_ = kError;
        if (!(reader.read(word) && word.size() >0 && digits::convert<uint32_t>(word.c_str(), bytes, 10) && bytes < kMaxValueSize)) request.code_ = kError;

        buffer.skip(end - start + 2);
        if (request.code_ != kError)  //todo overflow
        {
          //request.code_ = kSucc;
          request.key_ = key;
          request.flags_ = flags;
          request.exptime_ = exptime;
          request.bytes_ = bytes;
          request.state_ = kLineData;
          break;
        }
        else
        {
          request.state_ = kLineInit;
          return true;
        }
      }
      case kLineData:
      {
        
        if (buffer.readable() < request.bytes_ + 2) return false;
        const char* end = buffer.find(request.bytes_, CRLF);
        if (NULL == end) return false;
        if (end - buffer.beginRead() != request.bytes_)  //todo overflow
        {
          buffer.skip(end - buffer.beginRead() + 2);
          request.code_ = kError;
          request.state_ = kLineInit;
          return true;
        }
        else
        {
          request.value_ = string(buffer.beginRead(), end - buffer.beginRead()); //move
          buffer.skip(end - buffer.beginRead() + 2);
          request.code_ = kSucc;
          request.state_ = kLineInit;
          return true;
        }
      }
      default:
      {
        LOG_ERROR << "wont run here! state = " << request.state_ ;
        return true;
      }
    }
  }
  return false;
};

}
}