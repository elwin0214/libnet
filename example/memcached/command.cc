#include "command.h"
#include <strings.h>
#include <libnet/digits.h>
#include <assert.h>

void TextStoreCommand::append(Buffer& buffer)
{
  buffer.append(name_);
  buffer.append(" ");
  buffer.append(key_);
  char buf[32];
  ::bzero(buf, sizeof(buf));
  snprintf(buf, sizeof(buf), " %d %d %d", flags_, exptime_, bytes_);
  buffer.append(buf);
  buffer.append("\r\n");
  buffer.append(value_);
  buffer.append("\r\n");
};

bool TextStoreCommand::parse(Buffer& buffer)
{
  if (buffer.readable() < 2) return false;
  const char* p = buffer.find("\r\n");
  if (NULL == p) return false;
  if (isError(buffer, p)) return true;

  desc_ = std::string(buffer.beginRead(), p - buffer.beginRead());
  buffer.moveReadIndex(p + 2 - buffer.beginRead());
  if (desc_ == "STORED")
  {
    code_ = kSucc;
  }
  else if (desc_ == "NOT_STORED")
    code_ = kFail;
  else
    code_ = kError;
  return true;
};

void GetCommand::append(Buffer& buffer)
{
  buffer.append(name_);
  buffer.append(" ");
  buffer.append(key_);
  buffer.append("\r\n");
};

bool GetCommand::parseValueLine(Buffer& buffer, const char* crlf)
{
  const char* blank = buffer.find(" ");//VALUE <key> <flags> <bytes>\r\n
  if (NULL == blank)
  {
    code_ = kError;
    return false;
  }
  buffer.moveReadIndex(blank + 1 - buffer.beginRead());
  bool process = true;
  for (int index = 1; index <= 3; index++)
  {
    blank = buffer.find(" ");
    if (index <= 2 && NULL == blank)
    {
      code_ = kError;
      process = false;
      break;
    }
    if (index == 1)
    {
      getKey_ = std::string(buffer.beginRead(), blank - buffer.beginRead());
      buffer.moveReadIndex(blank + 1 - buffer.beginRead());
    }
    else if (index == 2)
    {
      std::string flags = std::string(buffer.beginRead(), blank - buffer.beginRead());
      buffer.moveReadIndex(blank + 1 - buffer.beginRead());
      digits::stringToDigit(flags.c_str(), &flags_);
    }
    else if (index == 3)
    {
      std::string bytes = std::string(buffer.beginRead(), crlf - buffer.beginRead());
      buffer.moveReadIndex(crlf + 1 - buffer.beginRead());
      digits::stringToDigit(bytes.c_str(), &bytes_);
    }
  }

  return process;
};

bool GetCommand::parse(Buffer& buffer)
{ 
  if (buffer.readable() < 2) return false;
  while(code_ == kInit)
  {
    const char* crlf = buffer.find("\r\n");
    if (NULL == crlf)
    {
      //code_ = kNeedMore;
      break;
    }
    if (isError(buffer, crlf)) return true;

    switch (state_)
    {
      case kLineInit:
      {
        if (buffer.at(0) == 'V')
          state_ = kLineValue;
        else if (buffer.at(0) == 'E' && buffer.at(1) == 'N')
          state_ = kLineEnd;
        else
        {
          buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
          code_ = kError;
        }
        break;
      }
      case kLineEnd:
      {
        buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
        code_ = kSucc;
        break;
      }
      case kLineValue: //VALUE <key> <flags> <bytes>\r\n
      {
        bool parse = parseValueLine(buffer, crlf);
        buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
        if (!parse)
          code_ = kError;
        else
          state_ = kLineData;
        break;
      }
      case kLineData:
      {
        if (buffer.readable() >= bytes_ + 2  && buffer.at(bytes_) == '\r' && buffer.at(bytes_ + 1) == '\n')
        {
          result_ = std::string(buffer.beginRead(), bytes_);
          state_ = kLineInit;
        }
        else
          code_ = kError;
        buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
        break;
      }
    }
  }
  return code_ != kInit;
};

void DeleteCommand::append(Buffer& buffer)
{
  buffer.append(name_);
  buffer.append(" ");
  buffer.append(key_);
  buffer.append("\r\n");
};

bool DeleteCommand::parse(Buffer& buffer)
{
  if (buffer.readable() < 2) return false;
  const char* crlf = buffer.find("\r\n");
  if (NULL == crlf) return false;
  if (isError(buffer, crlf)) return true;

  if (buffer.at(0) == 'D' && buffer.at(1) == 'E')//DELETED\r\n
  {
    code_ = kSucc;
  }
  else if(buffer.at(0) == 'N' && buffer.at(1) == 'O')//NOT_FOUND\r\n
  {
    code_ = kFail;
  }
  else
    code_ = kError;

  buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
  return true;
};

void CountCommand::append(Buffer& buffer)
{
  buffer.append(name_);
  buffer.append(" ");
  buffer.append(key_);
  buffer.append(" ");
  char buf[16];
  int n = sprintf(buf, "%d", value_);
  assert(n > 0);
  buffer.append(buf, n);
  buffer.append("\r\n");
};

bool CountCommand::parse(Buffer& buffer)
{
  if (buffer.readable() < 2) return false;
  const char* crlf = buffer.find("\r\n");
  if (NULL == crlf) return false;
  if (isError(buffer, crlf)) return true;

  if(buffer.at(0) == 'N' && buffer.at(1) == 'O')//NOT_FOUND\r\n
  {
    code_ = kFail;
  }
  else
  {
    result_ = std::string(buffer.beginRead(), crlf - buffer.beginRead());
    code_ = kSucc;
  }
  buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
  return true;
};





