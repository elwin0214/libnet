#include <libnet/logger.h>
#include <libnet/current_thread.h>
#include <assert.h>
#include <iostream>
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
  double l = 1000000 * 1.0 / 17;
  LOG_SYSFATAL <<  l << " error! ";
};

int main()
{
  LOG_WARN << (1000000 * 1.0 / 12) << "test " << thread::currentTid();
  std::cout << thread::currentTid() << std::endl;
  sys_error();
  sys_fatal();



  return 0;
}