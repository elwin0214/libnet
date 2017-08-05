#ifndef __LIBNET_EXCEPTION_H__
#define __LIBNET_EXCEPTION_H__

#include <errno.h>
#include <string.h>
#include <string>
#include <exception>
#include <libnet/nocopyable.h>

namespace libnet
{
class Exception : public std::exception
{
public:
  Exception(const char* str);
  Exception(const std::string& str);

  const char* message() const { return message_.c_str();}
  const char* stackTrace() const { return stack_.c_str(); }

private:
  void fillStackTrace();

private:
  std::string message_;
  std::string stack_;
};
}


#endif