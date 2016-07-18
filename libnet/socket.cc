#include <errno.h>
#include <strings.h>
#include "logger.h"
#include "socket.h"
#include "socket_ops.h"

namespace libnet
{

int Socket::bind(InetAddress &localaddr)
{
  return sockets::bind(fd_, localaddr.getSockAddrIn());
};

int Socket::listen(int backlog)
{
  return sockets::listen(fd_, backlog);
};

int Socket::accept(InetAddress *addr)
{
  sockaddr_in addr_in;
  ::bzero(&addr_in, sizeof(addr_in));
  int fd = sockets::accept(fd_, &addr_in); 
  addr->setSockAddrIn(addr_in);
  return fd;
};


int Socket::read(Buffer &buffer)
{
  size_t len = buffer.writable();
  ssize_t n = sockets::read(fd_, buffer.cur(), len);
  LOG_TRACE << "read return-" << n ;
  if (n <= 0) return n; //todo errno
  if (n > 0) 
    buffer.moveWriteIndex(n);
  return n;
};

int Socket::write(Buffer &buffer)
{
  int len = buffer.readable();
  ssize_t n = sockets::write(fd_, buffer.cur(), len);
  if (n <= 0) return n; //todo errno
  LOG_DEBUG << "buffer-" << buffer.toString() << ", len-" << len  ;
  buffer.moveReadIndex(n);
  return n;
};

int Socket::write(const CString &cstring)
{
  int len = cstring.length();
  ssize_t n = sockets::write(fd_, cstring.data(), len);
  if (n <= 0) return n; //todo errno
    //buffer.moveReadIndex(n);
  return n;
};

void Socket::setReuseAddr()
{
  sockets::setResuseAddr(fd_);
};

void Socket::setNoBlocking()
{
  sockets::setNoBlocking(fd_);
};

void Socket::shutdownWrite()
{
  LOG_TRACE << "fd-" << fd_ ;
  sockets::shutdownWrite(fd_);
};

Socket::~Socket()
{
  sockets::close(fd_);
};

}