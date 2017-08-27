#include <assert.h>
#include <libnet/digits.h>
#include <libnet/logger.h>
#include <libnet/mc/request_codec.h>
#include <libnet/mc/reader.h>
#include <libnet/mc/mcode.h>

namespace mc
{
namespace msg
{
using namespace libnet;
using namespace std;
static const size_t kMaxKeySize = 255;
static const size_t kMaxValueSize = 65535;
void RequestCodec::encode(Message& message, Buffer& buffer)
{
  Data& request = message.data_;
  if (request.op() == Opt::kSet || request.op() == Opt::kAdd || request.op() == Opt::kReplace)
  {
    buffer.append(request.op_name());
    buffer.append(" ");
    buffer.append(request.key_);
    char buf[128];
    ::bzero(buf, sizeof(buf));
    snprintf(buf, sizeof(buf), " %d %d %lu", request.flags_, request.exptime_, request.value_.size());
    LOG_TRACE << "key "<< request.key_ << " line " << buf ;
    buffer.append(buf);
    buffer.append(CRLF);
    buffer.append(request.value_);
    buffer.append(CRLF);
  }
  else if(request.op() == Opt::kGet)
  { 
    buffer.append(request.op_name());
    buffer.append(" ");
    buffer.append(request.key_);
    buffer.append(CRLF);
  }
  else if(request.op() == Opt::kDelete)
  {
    buffer.append(request.op_name());
    buffer.append(" ");
    buffer.append(request.key_);
    buffer.append(CRLF);
  }
  else if(request.op() == Opt::kIncr || request.op() == Opt::kDecr)
  {
    buffer.append(request.op_name());
    buffer.append(" ");
    buffer.append(request.key_);
    buffer.append(" ");
    char buf[16];
    int n = sprintf(buf, "%d", request.count_);
    assert(n > 0);
    buffer.append(buf, n);
    buffer.append(CRLF);
  }
  else if (request.op() == Opt::kVer)
  {
    buffer.append(request.op_name());
    buffer.append(CRLF);
  }
  else
  {
    assert(false);
  }
};


bool RequestCodec::decode(Message& message, Buffer& buffer)
{ 
  Stat& stat = message.stat_;
  Data& data = message.data_;
  const char* end = NULL;
  if (data.op() == kNo)
  {
    end = buffer.find(CRLF);
    if (NULL == end) return false; 
    buffer.trim();
    if (buffer.startWiths("get")) data.op_ = kGet;
    else if (buffer.startWiths("set")) { data.op_ = kSet; stat.line_ = kLineCmd; }
    else if (buffer.startWiths("replace")) { data.op_ = kReplace; stat.line_ = kLineCmd; }
    else if (buffer.startWiths("add")) { data.op_ = kAdd; stat.line_ = kLineCmd; }
    else if (buffer.startWiths("delete")) data.op_ = kDelete;
    else if (buffer.startWiths("incr")) data.op_ = kIncr;
    else if (buffer.startWiths("decr")) data.op_ = kDecr;
    else if (buffer.startWiths("version"))
    {
      stat.code_ = kSucc;
      data.op_ = kVer;
      buffer.skip(end - buffer.beginRead() + 2);
      return true;
    }
    else if (buffer.startWiths("quit"))
    {
      stat.code_ = kSucc;
      data.op_ = kQuit;
      buffer.skip(end - buffer.beginRead() + 2);
      return true;
    }
    else
    {
      buffer.skip(end - buffer.beginRead() + 2);
      stat.code_ = kError;
      return true;
    }
    LOG_TRACE << "op = " << data.op_ ;
  }

  if (data.op_ == kGet) return decodeSimple(message, buffer);
  if (data.op_ == kSet) return decodeStore(message, buffer, end);
  if (data.op_ == kReplace) return decodeStore(message, buffer, end);
  if (data.op_ == kAdd) return decodeStore(message, buffer, end);
  if (data.op_ == kDelete) return decodeSimple(message, buffer);
  if (data.op_ == kIncr || data.op_ == kDecr) return decodeCount(message, buffer);
  else
  {
    LOG_ERROR << "unknow op!"; 
    return true;
  }
};

bool RequestCodec::decodeSimple(Message& message, Buffer& buffer)
{
  Stat& stat = message.stat_;
  Data& data = message.data_;
  const char* end = buffer.find(CRLF);
  if (NULL == end) return false;
  const char* start = buffer.beginRead();
  Reader reader(start, end, ' ');
  string word;
  if (!(reader.read(word))) stat.code_ = kError;
  if (!reader.read(word)) stat.code_ = kError;

  if (stat.code_ != kError)
  {
    stat.code_ = kSucc;
    data.key_ = word;
  }
  buffer.skip(end - start + 2);
  return true;  
};

bool RequestCodec::decodeCount(Message& message, Buffer& buffer)
{
  Stat& stat = message.stat_;
  Data& data = message.data_;

  const char* end = buffer.find(CRLF);
  if (NULL == end) return false;
  const char* start = buffer.beginRead();
  Reader reader(start, end, ' ');
  string word;
  uint32_t count;
  if (!(reader.read(word))) stat.code_ = kError;
  if (!reader.read(word)) stat.code_ = kError;
  if (!(reader.read(word) && digits::convert<uint32_t>(word.c_str(), count, 10)))stat.code_ = kError;

  if (stat.code_ != kError)
  {
    stat.code_ = kSucc;
    data.key_ = word;
    data.count_ = count;
  }
  buffer.skip(end - start + 2);
  return true; 
};

bool RequestCodec::decodeStore(Message& message, Buffer& buffer, const char* cmd_end)
{
  Data& data = message.data_;
  Stat& stat = message.stat_;

  while (stat.code_ == kInit)
  {
    switch(stat.line_)
    {
      case kLineCmd:
      {
        const char* end = cmd_end;//buffer.find(CRLF);
        if (NULL == end) return false;
        const char* start = buffer.beginRead();
        Reader reader(start, end, ' ');
        uint16_t flags;
        uint32_t bytes;
        uint32_t exptime;
        string key;
        string word;
        if (!reader.read(word)) stat.code_ = kError;
        if (!(reader.read(key) && key.size() <= kMaxKeySize)) stat.code_ = kError;
        if (!(reader.read(word) && word.size() >0 && digits::convert<uint16_t>(word.c_str(), flags, 10))) stat.code_ = kError;
        if (!(reader.read(word) && word.size() >0 && digits::convert<uint32_t>(word.c_str(), exptime, 10))) stat.code_ = kError;
        if (!(reader.read(word) && word.size() >0 && digits::convert<uint32_t>(word.c_str(), bytes, 10) && bytes < kMaxValueSize)) stat.code_ = kError;

        buffer.skip(end - start + 2);
        if (stat.code_ != kError)  //todo overflow
        {
          //request.code_ = kSucc;
          data.key_ = key;
          data.flags_ = flags;
          data.exptime_ = exptime;
          data.bytes_ = bytes;
          stat.line_ = kLineData;
          break;
        }
        else
        {
          stat.line_ = kLineInit;
          return true;
        }
      }
      case kLineData:
      {
        
        if (buffer.readable() < data.bytes_ + 2) return false;
        const char* end = buffer.find(data.bytes_, CRLF);
        if (NULL == end) return false;
        if (end - buffer.beginRead() != data.bytes_)  //todo overflow
        {
          buffer.skip(end - buffer.beginRead() + 2);
          stat.code_ = kError;
          stat.line_ = kLineInit;
          return true;
        }
        else
        {
          data.value_ = string(buffer.beginRead(), end - buffer.beginRead()); //move
          buffer.skip(end - buffer.beginRead() + 2);
          stat.code_ = kSucc;
          stat.line_ = kLineInit;
          return true;
        }
      }
      default:
      {
        LOG_ERROR << "wont run here! state = " << stat.line_ ;
        return true;
      }
    }
  }
  return false;
};

}
}