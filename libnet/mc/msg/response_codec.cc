#include <assert.h>
#include <libnet/digits.h>
#include <libnet/mc/response_codec.h>
#include <libnet/mc/reader.h>
#include <libnet/mc/mcode.h>

using namespace libnet;
namespace mc
{
namespace msg
{
void ResponseCodec::encode(Message& message, Buffer& buffer)
{
  Data& data = message.data_;
  Stat& stat = message.stat_;

  if (stat.code_ == kError)
  {
    buffer.append("ERROR\r\n");
    return;
  }
  else
  {
    if (data.op_ == Opt::kGet)
    {
      if (stat.code_ == kFail)
      {
        buffer.append("END\r\n");
        return;
      }
      else if(stat.code_ == kSucc)
      {
        buffer.append("VALUE ");
        buffer.append(data.key_);
        buffer.append(" ");

        char buf[128];
        ::bzero(buf, sizeof(buf));
        snprintf(buf, sizeof(buf), "%u %u\r\n", data.flags_, data.exptime_);
        buffer.append(buf);
        buffer.append(data.value_);
        buffer.append(CRLF);
        buffer.append("END");
        buffer.append(CRLF);
      }
    }
    else if (data.op_ == Opt::kSet || data.op_ == Opt::kAdd || data.op_ == Opt::kReplace)
    {
      if (stat.code_ == kSucc)
      {
        buffer.append("STORED\r\n");
        return;
      }
      else if (stat.code_ == kFail)
      {
        buffer.append("NOT_STORED\r\n");
        return;
      }
    }
    else if (data.op_ == Opt::kDelete)
    {
      if (stat.code_ == kSucc)
      {
        buffer.append("DELETED\r\n");
        return;
      }
      else if (stat.code_ == kFail)
      {
        buffer.append("NOT_FOUND\r\n");
        return;
      }
    }
    else if (data.op_ == Opt::kIncr || data.op_ == Opt::kDecr)
    {
      if (stat.code_ == kSucc)
      {
        char buf[32];
        ::bzero(buf, sizeof(buf));
        snprintf(buf, sizeof(buf), "%d\r\n", data.count_);
        buffer.append(buf);
        return;
      }
    }
    else if (data.op_ == Opt::kVer)
    {
      buffer.append(data.value_);
      buffer.append(CRLF);
      return;
    }
    else
    {
      LOG_ERROR << "unknow op " << data.op_ ;
    }
  }

};

bool ResponseCodec::decode(Message& message, Buffer& buffer)
{
  Data& data = message.data_;
  //Stat& stat = message.stat_;

  if (data.op_ == Opt::kSet || data.op_ == Opt::kAdd || data.op_ == Opt::kReplace)
  {
    return decodeStore(message, buffer);
  }
  else if (data.op_ == Opt::kGet)
  {
    return decodeGet(message, buffer);
  }
  else if (data.op_ == Opt::kDelete)
  {
    return decodeDelete(message, buffer);
  }
  else if (data.op_ == Opt::kIncr || data.op_ == Opt::kDecr)
  {
    return decodeCount(message, buffer);
  }
  else if (data.op_ == Opt::kVer)
  {
    const char* p = buffer.find(CRLF);
    if (NULL == p) return false;
    std::string line = std::string(buffer.beginRead(), p - buffer.beginRead());
    message.data_.value_ = std::move(line);
    buffer.skip(p + 2 - buffer.beginRead());
    return true;
  }
  else
  {
    assert(false);
    return true;
  }
};

bool ResponseCodec::decodeStore(Message& message, Buffer& buffer)
{
  Data& data = message.data_;
  Stat& stat = message.stat_;

  if (buffer.readable() < 2) return false;
  const char* p = buffer.find(CRLF);
  if (NULL == p) return false;
  bool error = buffer.startWiths("ERROR") || buffer.startWiths("CLIENT_ERROR") || buffer.startWiths("SERVER_ERROR");
  if (error)
  {
    buffer.skip(p + 2 - buffer.beginRead());
    stat.code_ = kError;
    return true;
  }
  std::string line = std::string(buffer.beginRead(), p - buffer.beginRead());
  buffer.skip(p + 2 - buffer.beginRead());
  if (line == "STORED")
  {
    stat.code_ = kSucc;
  }
  else if (line == "NOT_STORED")
  {
    stat.code_ = kFail;
  }
  else
  {
    stat.code_ = kError;
  }
  return true;
};

bool ResponseCodec::decodeGet(Message& message, Buffer& buffer)
{
  Data& data = message.data_;
  Stat& stat = message.stat_;

  if (buffer.readable() < 2) return false;
  while(stat.code_ == kInit)
  {
    const char* crlf = NULL;
    crlf = buffer.find(CRLF);
    if (NULL == crlf) return false;
    switch (stat.line_)
    {
      case kLineInit:
      {
        if (buffer.startWiths("ERROR") || buffer.startWiths("CLIENT_ERROR") || buffer.startWiths("SERVER_ERROR"))
        {
          buffer.skip(crlf + 2 - buffer.beginRead());
          stat.code_ = kError;
          return true;
        }
        else if (buffer.at(0) == 'V')  // todo optimize
          stat.line_ = kLineValue;
        else if (buffer.at(0) == 'E' && buffer.at(1) == 'N')
          stat.line_ = kLineEnd;
        else
        {
          buffer.skip(crlf + 2 - buffer.beginRead());
          stat.code_ = kError;
          return true;
        }
        break;
      }
      case kLineEnd:
      {
        buffer.skip(crlf + 2 - buffer.beginRead());
        stat.code_ = kSucc;
        return true;
      }
      case kLineValue: //VALUE <key> <flags> <bytes>\r\n
      {
        Reader reader(buffer.beginRead(), crlf, ' ');
        uint16_t flags;
        uint32_t bytes;
        std::string value;
        if (!(reader.read(value) && value == "VALUE")) stat.code_ = kError;
        if (!reader.read(value)) stat.code_ = kError;
        if (!(reader.read(value) && value.size() >0 && digits::convert<uint16_t>(value.c_str(), flags, 10))) stat.code_ = kError;
        if (!(reader.read(value) && value.size() >0 && digits::convert<uint32_t>(value.c_str(), bytes, 10))) stat.code_ = kError;
        buffer.skip(crlf + 2 - buffer.beginRead());
        if (stat.code_ != kError){
          data.flags_ = flags;
          data.bytes_ = bytes;
          stat.line_ = kLineData;
          break;
        }
        else
        {
          return true;
        }
      }
      case kLineData:
      {
        if (buffer.readable() >= data.bytes_ + 2  && buffer.at(data.bytes_) == '\r' && buffer.at(data.bytes_ + 1) == '\n')
        {
          data.value_ = std::string(buffer.beginRead(), data.bytes_); //move?
          stat.line_ = kLineInit;
          buffer.skip(crlf + 2 - buffer.beginRead());
          break;
        }
        else
        {
          buffer.skip(crlf + 2 - buffer.beginRead());
          stat.code_ = kError;
          return true;
        }
      }
    }
  }
  return stat.code_ != kInit;
};

bool ResponseCodec::decodeDelete(Message& message, Buffer& buffer)
{
  Data& data = message.data_;
  Stat& stat = message.stat_;

  if (buffer.readable() < 2) return false;
  const char* crlf = buffer.find("\r\n");
  if (NULL == crlf) return false;
  if (buffer.at(0) == 'D' && buffer.at(1) == 'E')//DELETED\r\n
  {
    stat.code_ = kSucc;
  }
  else if(buffer.at(0) == 'N' && buffer.at(1) == 'O')//NOT_FOUND\r\n
  {
    stat.code_ = kFail;
  }
  else
  {
    stat.code_ = kError;
  }
  buffer.skip(crlf + 2 - buffer.beginRead());
  return true;
};

bool ResponseCodec::decodeCount(Message& message, Buffer& buffer)
{
  Data& data = message.data_;
  Stat& stat = message.stat_;

  if (buffer.readable() < 2) return false;
  const char* crlf = buffer.find("\r\n");
  if (NULL == crlf) return false;
  if (buffer.startWiths("ERROR") || buffer.startWiths("CLIENT_ERROR") || buffer.startWiths("SERVER_ERROR"))
  {
    buffer.skip(crlf + 2 - buffer.beginRead());
    stat.code_ = kError;
    return true;
  }

  if(buffer.at(0) == 'N' && buffer.at(1) == 'O')//NOT_FOUND\r\n
  {
    stat.code_ = kFail;
  }
  else
  {
    std::string value = std::string(buffer.beginRead(), crlf - buffer.beginRead());
    if (digits::convert<uint32_t>(value.c_str(), data.count_, 10))
    {
      stat.code_ = kSucc;
    }
    else 
      stat.code_ = kError;
  }
  buffer.skip(crlf + 2 - buffer.beginRead());
  return true;
};

}
}