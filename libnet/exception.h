#ifndef __LIBNET_LOG_LOGGER_H__
#define __LIBNET_LOG_LOGGER_H__

#include <errno.h>
#include <string.h>
#include <string>
#include "nocopyable.h"

namespace libnet
{
class Exception
{
public:
  Exception();
  Exception(const Exception& e); //move constructor?
  Exception& operator=(Exception e);
  std::string& toString(){return stack_; }

private:
  fillStackTrace();

private:
  std::string stack_;
};
}

class ConvertException : public Exception
{

};

#endif