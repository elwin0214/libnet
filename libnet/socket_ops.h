#ifndef __LIBNET_SOCKETS_OPS_H__
#define __LIBNET_SOCKETS_OPS_H__

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

namespace libnet
{
namespace sockets
{

void createSocketPair(int pair[]);

void createPipe(int fds[]);

int createSocketFd();

void setNoBlocking(int fd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);

struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);

int bind(int fd, const struct sockaddr_in &addr);

int listen(int fd, int backlog);

int accept(int fd, struct sockaddr_in *peerAddr);

int connect(int fd, const struct sockaddr_in &serverAddr);

ssize_t read(int sockfd, void *buf, size_t count);

ssize_t write(int sockfd, const void *buf, size_t count);

void close(int sockfd);

void convert(const char *str, uint16_t port, sockaddr_in *addr);

std::string getIp(sockaddr_in &addr);

uint16_t getPort(sockaddr_in &addr);

void setResuseAddr(int fd);

void shutdownWrite(int fd);

int getSocketError(int fd);

}
}

#endif