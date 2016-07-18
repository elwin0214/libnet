#ifndef __LIBNET_DIGITS_H__
#define __LIBNET_DIGITS_H__

#include <stdio.h>
#include <stdlib.h>

namespace libnet
{
namespace digits
{
  
inline int stringToDigit(const char* str, int *num)
{
  return sscanf(str,"%d", num);
};

inline int xstringToDigit(const char* str, int *num)
{
  return sscanf(str,"%x", num);
};


inline int digitToString(int num, char* str)
{
  return sprintf(str,"%d", num);
};

inline int digitToXstring(int num, char* str)
{
  return sprintf(str,"%x", num);
};

}
}

#endif