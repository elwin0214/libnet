#ifndef __LIBNET_INETADDRESS_H__
#define __LIBNET_INETADDRESS_H__

#include <libnet/nocopyable.h>
#include <string>
#include <netinet/in.h>

namespace libnet
{

class InetAddress
{
public:
  InetAddress();

  InetAddress(const char *host, uint16_t port);

  InetAddress(const std::string& host, uint16_t port);

  const sockaddr_in& getSockAddrIn() const { return addr_; }

  void setSockAddrIn(const sockaddr_in &addrin);
  
  std::string getIp();

  uint16_t getPort();

private:
  struct sockaddr_in addr_; 
};


}

#endif