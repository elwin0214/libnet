#ifndef __LIBNET_DIGITS_H__
#define __LIBNET_DIGITS_H__

#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

namespace libnet
{
namespace digits
{

inline int stringToDigit(const char* str, unsigned int *num)
{
  int r = sscanf(str,"%d", num);
  if (r < 0)
    LOG_SYSERROR << "str=" << str ;
  return r;
};

inline int stringToDigit(const char* str, unsigned long *num)
{
  int r = sscanf(str,"%ld", num);
  if (r < 0)
    LOG_SYSERROR << "str=" << str ;
  return r;
};

inline int xstringToDigit(const char* str, unsigned int *num)
{
  int r = sscanf(str,"%x", num);
  if (r < 0)
    LOG_SYSERROR << "str=" << str ;
  return r;
};

inline int xstringToDigit(const char* str, unsigned long *num)
{
  int r = sscanf(str,"%lx", num);
  if (r < 0)
    LOG_SYSERROR << "str=" << str ;
  return r;
};


inline int digitToString(uint32_t num, char* str)
{
  int r = sprintf(str,"%d", num);
  if (r < 0)
    LOG_SYSERROR << "num=" << num ;
  return r;
};

inline int digitToXstring(uint32_t num, char* str)
{
  int r = sprintf(str,"%x", num);
  if (r < 0)
     LOG_SYSERROR << "num=" << num ;
  return r;
};

}
}

#endif