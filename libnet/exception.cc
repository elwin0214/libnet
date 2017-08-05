#include <execinfo.h>
#include <string>
#include <libnet/exception.h>

namespace libnet
{

Exception::Exception(const char* str):message_(str)
{
  fillStackTrace();
};

Exception::Exception(const std::string& str):message_(str)
{
  fillStackTrace();
};

void Exception::fillStackTrace()
{ 
  const int len = 128;
  void* buffer[len];
  int nptrs = ::backtrace(buffer, len);
  char** strings = ::backtrace_symbols(buffer, nptrs);

  if (strings)
  {
    for (int i = 0; i < nptrs; i++)
    {
      stack_.append(strings[i]);
      stack_.append("\n");
    }
  }
};

}