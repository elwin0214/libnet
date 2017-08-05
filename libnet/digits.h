#ifndef __LIBNET_DIGITS_H__
#define __LIBNET_DIGITS_H__

#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <libnet/logger.h>

namespace libnet
{
namespace digits
{

template<typename T>
bool convert(const std::string& str, T& result, int base) //error
{
  return convert<T>(str.c_str(), result, base);
};

template<typename T>
bool convert(const char* str, T& result, int base)
{
  int64_t value = 0; 
  try
  {
    value = std::stoll(str, nullptr, base);  
  }
  catch(...)
  {
    return false;
  }
  if (value < 0 || value > std::numeric_limits<T>::max())
  {
    return false;
  }
  result = static_cast<T>(value);
  return true;
};

inline int digitToXstring(size_t num, char* str)
{
  int r = sprintf(str,"%zx", num);
  if (r < 0)
     LOG_SYSERROR << "num=" << num ;
  return r;
};

}
}

#endif