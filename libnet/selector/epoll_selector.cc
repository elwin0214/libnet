#include <strings.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <libnet/logger.h>
#include <libnet/channel.h>
#include "epoll_selector.h"

namespace libnet
{
namespace selector
{

static int kNew = -1;
static int kAdded = 0;
static int kDeleted = 1;
static int kDefaultEvents = 1;


EpollSelector::EpollSelector(EventLoop* loop)
    : Selector(loop),
      epfd_(::epoll_create1(0)),
      epollEvents_(kDefaultEvents)
{
  assert(EPOLLIN == POLLIN);
  assert(EPOLLPRI == POLLPRI);
  assert(EPOLLOUT == POLLOUT);
  assert(EPOLLRDHUP == POLLRDHUP);
  assert(EPOLLERR == POLLERR);
  assert(EPOLLHUP == POLLHUP);
};

EpollSelector::~EpollSelector()
{
  LOG_TRACE << "~EpollSelector()" ;
  ::close(epfd_);
};

void EpollSelector::selectNow(ChannelList& activeChannles)
{
  select(0, activeChannles);
};

void EpollSelector::select(ChannelList& activeChannles)
{
  select(-1, activeChannles);
};

void EpollSelector::select(int timeoutMs, ChannelList& activeChannles)
{
  int num = ::epoll_wait(epfd_, &*epollEvents_.begin(), epollEvents_.size(), timeoutMs);
  if (num < 0)
  {
    LOG_SYSERROR << "timeout = "<< timeoutMs ;
    return;
  }
  if (num == 0)
  {
    LOG_TRACE << "timeout = "<< timeoutMs;
    return;
  }
  
  for (int i = 0; i < num; i++)
  {
    Channel* channel = static_cast<Channel*>(epollEvents_[i].data.ptr);
    int events = epollEvents_[i].events;
    int revents = 0;
    if ((events & POLLHUP) && !(events & POLLIN))  // will get POLLHUP event after shutdownWrite
    {
      revents |= Channel::kErrorEvent;
    }
    if (events & (POLLNVAL | POLLERR))
    {
      revents |= Channel::kErrorEvent;
    }
    #ifdef __APPLE__
    if (events & (POLLIN | POLLPRI))
    #else
    if (events & (POLLIN | POLLPRI | POLLRDHUP))
    #endif
    {
      revents |= Channel::kReadEvent;
    }
    if (events & POLLOUT)
    {
      revents |= Channel::kWriteEvent;
    }
    LOG_TRACE << "fd = " << channel->fd() << " revents = " << revents << " epoll_revents = " << events;
    channel->setRevents(revents);
    activeChannles.push_back(channel);
  }
  int size = epollEvents_.size();
  if (num >= size)
  {
    LOG_TRACE << "size=" << size << " resize ";
    epollEvents_.resize(epollEvents_.size() * 2);
  }
};


void EpollSelector::updateChannel(int op, Channel* channel)
{
  
  int fd = channel->fd();
  struct epoll_event epollEvent;
  ::bzero(&epollEvent, sizeof(epollEvent));
  int events = channel->events();
  if (events & Channel::kReadEvent)
  {
    epollEvent.events |= EPOLLIN | POLLPRI;
  }
  if (events & Channel::kWriteEvent)
  {
    epollEvent.events |= EPOLLOUT;
  }
  epollEvent.data.fd = fd;
  epollEvent.data.ptr = channel;
  LOG_TRACE << "op=" << op << " fd=" << fd << " events=" << events <<" pevents=" << epollEvent.events;
  int r = ::epoll_ctl(epfd_, op, fd, &epollEvent);
  if (r < 0)
  {
    LOG_SYSERROR << "fd=" << fd << " op=" << op;
  }
};

void EpollSelector::updateChannel(Channel* channel)
{
  int fd = channel->fd();
  if (channel->index() == kNew || channel->index() == kDeleted)
  {
    updateChannel(EPOLL_CTL_ADD, channel);
    if (channel->index() == kNew)
    {
      channels_[fd] = channel;
    }
    channel->setIndex(kAdded);
  }
  else 
  {
    if (channel->events() == Channel::kNoneEvent)
    {
      updateChannel(EPOLL_CTL_DEL, channel);
      channel->setIndex(kDeleted);
    }
    else
    {
      updateChannel(EPOLL_CTL_MOD, channel);
    }
  }
};

void EpollSelector::removeChannel(Channel* channel)
{
  if (channel->index() == kAdded)
  {
    updateChannel(EPOLL_CTL_DEL, channel);
  }
  channels_.erase(channel->fd());
  channel->setIndex(kNew);
};

}
}