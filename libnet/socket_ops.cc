#include <errno.h> 
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "socket_ops.h"
#include "logger.h"

namespace libnet
{
namespace sockets
{
void createSocketPair(int pair[])
{
  if (0 > ::socketpair(AF_UNIX, 0, SOCK_STREAM, pair))
  {
    LOG_SYSFATAL << "createSocketPair!" ;
  }
};

void createPipe(int fds[])
{
  if (0 > ::pipe(fds))
  {
    LOG_SYSFATAL << "createPipe!" ;
  }
};

int createSocketFd()
{
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    LOG_SYSFATAL << "createSocketFd!" ;
  else
    LOG_TRACE << "fd=" << fd;
  return fd;
};

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr)
{
  return reinterpret_cast<const struct sockaddr*>(addr);
};

struct sockaddr* sockaddr_cast(struct sockaddr_in* addr)
{
  return reinterpret_cast<struct sockaddr*>(addr);
};

int bind(int fd, const struct sockaddr_in &addr)
{ 
    //
  int ret = ::bind(fd, (const struct sockaddr*)(&addr), static_cast<socklen_t>(sizeof addr));
  if (ret < 0)
  {
    LOG_SYSFATAL << "fd=" << fd  << "bind!" ;
  }
  return ret;
};

int listen(int fd, int backlog)
{
  int ret = ::listen(fd, backlog);
  if (ret < 0)
    LOG_SYSFATAL << "fd=" << fd <<"setResuseAddr!" ;
  else
    LOG_TRACE << "fd=" << fd;
  return ret;
};

int accept(int fd, struct sockaddr_in* peerAddr)
{
  socklen_t addrlen = static_cast<socklen_t>(sizeof *peerAddr);
  int cfd = ::accept(fd, sockaddr_cast(peerAddr), &addrlen);
  return cfd;
};

int connect(int fd, const struct sockaddr_in &serverAddr)
{
  socklen_t addrlen = static_cast<socklen_t>(sizeof(serverAddr));
  return ::connect(fd, sockaddr_cast(&serverAddr), addrlen);
};

ssize_t read(int fd, void *buf, size_t count)
{
  return ::read(fd, buf, count);
};

ssize_t write(int fd, const void *buf, size_t count)
{
  return ::write(fd, buf, count);
};

void close(int fd)
{
  if (::close(fd) < 0)
    LOG_SYSERROR << "sockfd=" << fd <<" close!" ;
  else
    LOG_DEBUG << "sockfd=" << fd ;
};

void convert(const char *host, uint16_t port, struct sockaddr_in *addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  ::inet_pton(AF_INET, host, &(addr->sin_addr));
};

std::string getIp(sockaddr_in &addr)
{
  char ip[32];
  ::inet_ntop(AF_INET, &(addr.sin_addr), ip, sizeof(ip));
  return ip;
};

uint16_t getPort(sockaddr_in &addr)
{
  return ntohs(addr.sin_port);
};

int getSocketError(int sockfd)
{
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof optval);

  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    return errno;
  }
  else
  {
    return optval;
  }
};

void setNoBlocking(int fd)
{
  int flags = ::fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int ret = ::fcntl(fd, F_SETFL, flags);
  if (ret < 0)
    LOG_SYSERROR << "sockfd=" << fd ;
};

}
}
