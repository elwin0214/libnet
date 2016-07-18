#include <cpptest.h>
#include <libnet/logger.h>
#include <assert.h>

using namespace libnet;
using namespace libnet::log;

void sys_error()
{
  errno = 48;
  LOG_SYSERROR << "error! ";
};

void sys_fatal()
{
  errno = 48;
  LOG_SYSFATAL << "error! ";
};

int main()
{
  sys_error();
  sys_fatal();
  return 0;
}