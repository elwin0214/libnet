#ifndef __LIBNET_SOCKET_H__
#define __LIBNET_SOCKET_H__

#include <libnet/nocopyable.h>
#include <libnet/buffer.h>
#include <libnet/inet_address.h>

namespace libnet
{

class Socket : public NoCopyable 
{

public:
  explicit Socket(int fd):fd_(fd)
  {
  }

  int fd(){ return fd_; }

  int bind(const InetAddress &addr);

  int listen(int backlog);
    
  int accept(InetAddress *addr);

  int read(Buffer &buffer);

  int write(Buffer &buffer);

  int write(const CString &cstring);
    
  void shutdownWrite();

  void setReuseAddr();

  void setNoBlocking();

  void setTcpNoDelay(bool on);

  void setKeepAlive(bool on);

  ~Socket();

private:
    int fd_;

};

}

#endif


