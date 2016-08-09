
#ifndef __LIBNET_SOCKET_H__
#define __LIBNET_SOCKET_H__

#include "nocopyable.h"
#include "buffer.h"
#include "inet_address.h"

namespace libnet
{

class Socket : public NoCopyable 
{

public:
  explicit Socket(int fd):fd_(fd)
  {
  }

  int fd(){ return fd_; }

  int bind(InetAddress &localaddr);

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


