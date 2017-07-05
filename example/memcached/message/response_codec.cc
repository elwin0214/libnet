#include <assert.h>
#include <libnet/digits.h>
#include "response_codec.h"
#include "reader.h"
#include "mcode.h"

using namespace libnet;
namespace memcached
{
namespace message
{
void ResponseCodec::encode(Response& response, Buffer& buffer)
{
  if (response.code_ == kError)
  {
    buffer.append("ERROR\r\n");
    return;
  }
  else
  {
    if (response.op_ == Opt::kGet)
    {
      if (response.code_ == kFail)
      {
        buffer.append("END\r\n");
        return;
      }
      else if(response.code_ == kSucc)
      {
        buffer.append("VALUE ");
        buffer.append(response.key_);
        buffer.append(" ");

        char buf[128];
        ::bzero(buf, sizeof(buf));
        snprintf(buf, sizeof(buf), "%u %u\r\n", response.flags_, response.exptime_);
        buffer.append(buf);
        buffer.append(response.value_);
        buffer.append(CRLF);
        buffer.append("END");
        buffer.append(CRLF);
      }
    }
    else if (response.op_ == Opt::kSet || response.op_ == Opt::kAdd || response.op_ == Opt::kReplace)
    {
      if (response.code_ == kSucc)
      {
        buffer.append("STORED\r\n");
        return;
      }
      else if (response.code_ == kFail)
      {
        buffer.append("NOT_STORED\r\n");
        return;
      }
    }
    else if (response.op_ == Opt::kDelete)
    {
      if (response.code_ == kSucc)
      {
        buffer.append("DELETED\r\n");
        return;
      }
      else if (response.code_ == kFail)
      {
        buffer.append("NOT_FOUND\r\n");
        return;
      }
    }
    else if (response.op_ == Opt::kIncr || response.op_ == Opt::kDecr)
    {
      if (response.code_ == kSucc)
      {
        char buf[32];
        ::bzero(buf, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d\r\n", response.count_);
        buffer.append(buf);
        return;
      }
    }
  }

};

bool ResponseCodec::decode(Response& response, Buffer& buffer)
{
  if (response.op_ == Opt::kSet || response.op_ == Opt::kAdd || response.op_ == Opt::kReplace)
  {
    return decodeStore(response, buffer);
  }
  else if (response.op_ == Opt::kGet)
  {
    return decodeGet(response, buffer);
  }
  else if (response.op_ == Opt::kDelete)
  {
    return decodeDelete(response, buffer);
  }
  else if (response.op_ == Opt::kIncr || response.op_ == Opt::kDecr)
  {
    return decodeCount(response, buffer);
  }
  else
  {
    assert(false);
    return true;
  }
};

bool ResponseCodec::decodeStore(Response& response, Buffer& buffer)
{
  if (buffer.readable() < 2) return false;
  const char* p = buffer.find(CRLF);
  if (NULL == p) return false;
  bool error = buffer.startWiths("ERROR") || buffer.startWiths("CLIENT_ERROR") || buffer.startWiths("SERVER_ERROR");
  if (error)
  {
    buffer.skip(p + 2 - buffer.beginRead());
    response.code_ = kError;
    return true;
  }
  std::string line = std::string(buffer.beginRead(), p - buffer.beginRead());
  buffer.skip(p + 2 - buffer.beginRead());
  if (line == "STORED")
  {
    response.code_ = kSucc;
  }
  else if (line == "NOT_STORED")
  {
    response.code_ = kFail;
  }
  else
  {
    response.code_ = kError;
  }
  return true;
};

bool ResponseCodec::decodeGet(Response& response, Buffer& buffer)
{
  if (buffer.readable() < 2) return false;
  while(response.code_ == kInit)
  {
    const char* crlf = NULL;
    crlf = buffer.find(CRLF);
    if (NULL == crlf) return false;
    switch (response.state_)
    {
      case kLineInit:
      {
        if (buffer.startWiths("ERROR") || buffer.startWiths("CLIENT_ERROR") || buffer.startWiths("SERVER_ERROR"))
        {
          buffer.skip(crlf + 2 - buffer.beginRead());
          response.code_ = kError;
          return true;
        }
        else if (buffer.at(0) == 'V')  // todo optimize
          response.state_ = kLineValue;
        else if (buffer.at(0) == 'E' && buffer.at(1) == 'N')
          response.state_ = kLineEnd;
        else
        {
          buffer.skip(crlf + 2 - buffer.beginRead());
          response.code_ = kError;
          return true;
        }
        break;
      }
      case kLineEnd:
      {
        buffer.skip(crlf + 2 - buffer.beginRead());
        response.code_ = kSucc;
        return true;
      }
      case kLineValue: //VALUE <key> <flags> <bytes>\r\n
      {
        Reader reader(buffer.beginRead(), crlf, ' ');
        uint16_t flags;
        uint32_t bytes;
        std::string value;
        if (!(reader.read(value) && value == "VALUE")) response.code_ = kError;
        if (!reader.read(value)) response.code_ = kError;
        if (!(reader.read(value) && value.size() >0 && digits::convert<uint16_t>(value.c_str(), flags, 10))) response.code_ = kError;
        if (!(reader.read(value) && value.size() >0 && digits::convert<uint32_t>(value.c_str(), bytes, 10))) response.code_ = kError;
        buffer.skip(crlf + 2 - buffer.beginRead());
        if (response.code_ != kError){
          response.flags_ = flags;
          response.bytes_ = bytes;
          response.state_ = kLineData;
          break;
        }
        else
        {
          return true;
        }
      }
      case kLineData:
      {
        if (buffer.readable() >= response.bytes_ + 2  && buffer.at(response.bytes_) == '\r' && buffer.at(response.bytes_ + 1) == '\n')
        {
          response.value_ = std::string(buffer.beginRead(), response.bytes_); //move?
          response.state_ = kLineInit;
          buffer.skip(crlf + 2 - buffer.beginRead());
          break;
        }
        else
        {
          buffer.skip(crlf + 2 - buffer.beginRead());
          response.code_ = kError;
          return true;
        }
      }
    }
  }
  return response.code_ != kInit;
};

bool ResponseCodec::decodeDelete(Response& response, Buffer& buffer)
{
  if (buffer.readable() < 2) return false;
  const char* crlf = buffer.find("\r\n");
  if (NULL == crlf) return false;
  if (buffer.at(0) == 'D' && buffer.at(1) == 'E')//DELETED\r\n
  {
    response.code_ = kSucc;
  }
  else if(buffer.at(0) == 'N' && buffer.at(1) == 'O')//NOT_FOUND\r\n
  {
    response.code_ = kFail;
  }
  else
  {
    response.code_ = kError;
  }
  buffer.skip(crlf + 2 - buffer.beginRead());
  return true;
};

bool ResponseCodec::decodeCount(Response& response, Buffer& buffer)
{
  if (buffer.readable() < 2) return false;
  const char* crlf = buffer.find("\r\n");
  if (NULL == crlf) return false;
  if (buffer.startWiths("ERROR") || buffer.startWiths("CLIENT_ERROR") || buffer.startWiths("SERVER_ERROR"))
  {
    buffer.skip(crlf + 2 - buffer.beginRead());
    response.code_ = kError;
    return true;
  }

  if(buffer.at(0) == 'N' && buffer.at(1) == 'O')//NOT_FOUND\r\n
  {
    response.code_ = kFail;
  }
  else
  {
    std::string value = std::string(buffer.beginRead(), crlf - buffer.beginRead());
    if (digits::convert<uint32_t>(value.c_str(), response.count_, 10))
    {
      response.code_ = kSucc;
    }
    else 
      response.code_ = kError;
  }
  buffer.skip(crlf + 2 - buffer.beginRead());
  return true;
};

}
}