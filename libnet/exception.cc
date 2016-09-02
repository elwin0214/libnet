#include <execinfo.h>
#include <string>
#include "exception.h"

namespace libnet
{
Exception::Exception()
{
  fillStackTrace();
};

Exception::Exception(const Exception& e)
{
  stack_ = e.stack_; //exception safe?
};

Exception& Exception::operator=(Exception e)
{
  swap(stack_, e.stack_);
};
//todo move constructor?
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