#include "command.h"
#include <strings.h>
#include <libnet/digits.h>

void SetCommand::append(Buffer& buffer)
{
  buffer.append(name_);
  buffer.append(" ");
  buffer.append(key_);
  char buf[32];
  ::bzero(buf, sizeof(buf));
  snprintf(buf, sizeof(buf), " %d %d %d ", flags_, exptime_, bytes_);
  buffer.append(buf);
  buffer.append("\r\n");
  buffer.append(value_);
  buffer.append("\r\n");
};

void SetCommand::parse(Buffer& buffer)
{
  const char* p = buffer.find("\r\n");
  if (NULL == p) 
  {
    code_ = kNeedMore;
    return;
  }
  result_ = std::string(buffer.beginRead(), p - buffer.beginRead());
  buffer.moveReadIndex(p + 2 - buffer.beginRead());
  code_ = kSucc;
};


void GetCommand::append(Buffer& buffer)
{
  buffer.append(name_);
  buffer.append(" ");
  buffer.append(key_);
  buffer.append("\r\n");
};

void GetCommand::parse(Buffer& buffer)
{
  if (line_ == 0)
  {
    const char* crlf = buffer.find("\r\n");
    if (NULL == crlf)
    {
      code_ = kNeedMore;
      return;
    }
    line_ = 1;

    const char* blank = buffer.rfind(crlf, ' ');
    if (blank == NULL)
    {
      code_ = kError;
      return;
    }
    std::string bytes = std::string(blank + 1, crlf - blank - 1);
    digits::stringToDigit(bytes.c_str(), &bytes_);
    buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
  }
  if (line_ == 1)
  {
    const char* crlf = buffer.find("\r\n");
    if (NULL == crlf)
    {
      code_ = kNeedMore;
      return;
    }
    line_ = 2;
    result_ = std::string(buffer.beginRead(), crlf - buffer.beginRead());
    buffer.moveReadIndex(crlf + 2 - buffer.beginRead());
    code_ = kNeedMore;
  }
  if (line_ == 2)
  {
    const char* crlf = buffer.find("\r\n");
    if (NULL == crlf)
    {
      code_ = kNeedMore;
      return;
    }
    code_ = kSucc;
  }
};

