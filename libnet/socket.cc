#include <errno.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
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
  if (fd > 0)
    addr->setSockAddrIn(addr_in);
  return fd;
};


int Socket::read(Buffer &buffer)
{
  size_t len = buffer.writable();
  if (len <= 0) buffer.makeRoom(128); 
  len = buffer.writable();
  ssize_t n = sockets::read(fd_, buffer.beginWrite(), len);
  LOG_TRACE << "read return=" << n ;
  if (n <= 0) return n; //todo errno
  if (n > 0) 
    buffer.moveWriteIndex(n);
  return n;
};

int Socket::write(Buffer &buffer)
{
  int len = buffer.readable();
  ssize_t n = sockets::write(fd_, buffer.beginRead(), len);
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
  return n;
};

void Socket::shutdownWrite()
{
  if(::shutdown(fd_, SHUT_WR) < 0)
    LOG_SYSERROR << "sockfd=" << fd_ <<" close!" ;
  else
    LOG_DEBUG << "sockfd=" << fd_ ;
};

void Socket::setReuseAddr()
{
  int val = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &val, static_cast<socklen_t>(sizeof(int))) < 0)
  {
    LOG_SYSERROR << "sockfd=" << fd_ ;
  }
};

void Socket::setNoBlocking()
{
  sockets::setNoBlocking(fd_);
};

void Socket::setTcpNoDelay(bool on)
{
  int val = on ? 1 : 0;
  if (setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &val, static_cast<socklen_t>(sizeof(int))) < 0)
  {
    LOG_SYSERROR << "sockfd=" << fd_  << " on=" << on ;
  }
};

void Socket::setKeepAlive(bool on)
{
  int val = on ? 1 : 0;
  if (setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &val, static_cast<socklen_t>(sizeof(int))) < 0)
  {
    LOG_SYSERROR << "sockfd=" << fd_  << " on=" << on ;
  }
};

Socket::~Socket()
{
  sockets::close(fd_);
};

}