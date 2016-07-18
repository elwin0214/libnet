#ifndef __LIBNET_CSTRING_H__
#define __LIBNET_CSTRING_H__
#include <string.h>

namespace libnet
{

class CString
{
public:
  CString(const char* data, size_t len)
    : data_(data),
      len_(len)
  {

  }

  CString(const char* data)
    : data_(data),
      len_(strlen(data))
  {

  }

  const char* data() const { return data_; }
  size_t length() const {return len_; }

private:
  const char *data_;
  size_t len_;

};

}
#endif