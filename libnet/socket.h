
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
  ~Socket();

  int fd(){ return fd_; }

  int bind(InetAddress &localaddr);

  int listen(int backlog);
    
  int accept(InetAddress *addr);

  int read(Buffer &buffer);

  int write(Buffer &buffer);

  int write(const CString &cstring);

  void setReuseAddr();

  void setNoBlocking();
    
  void shutdownWrite();

private:
    int fd_;

};

}

#endif


