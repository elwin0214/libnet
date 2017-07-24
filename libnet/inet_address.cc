#include "inet_address.h"
#include "socket_ops.h"
#include <string.h>

namespace libnet
{

InetAddress::InetAddress()
{
};

InetAddress::InetAddress(const char *host, uint16_t port)
{
  ::bzero(&addr_, sizeof(addr_));
  sockets::convert(host, port, &addr_);
};

InetAddress::InetAddress(const std::string& host, uint16_t port)
{
  ::bzero(&addr_, sizeof(addr_));
  sockets::convert(host.c_str(), port, &addr_);
};

void InetAddress::setSockAddrIn(const sockaddr_in &addrin)
{
  addr_ = addrin;
};

std::string InetAddress::getIp()
{
  return sockets::getIp(addr_);
};

uint16_t InetAddress::getPort()
{
  return sockets::getPort(addr_);
};

}
 