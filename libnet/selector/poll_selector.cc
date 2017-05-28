#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include "../logger.h"
#include "../channel.h"
#include "poll_selector.h"

namespace libnet
{
namespace selector
{
PollSelector::PollSelector(EventLoop* loop)
    : Selector(loop),
      pollfds_(),
      index_(0)
{

};

PollSelector::~PollSelector()
{

};

void PollSelector::updateChannel(Channel *channel)
{
  int events = channel->events();
  int fd = channel->fd();

  int pevents = 0;
  if (events & Channel::kReadEvent)
  {
    pevents |= POLLIN | POLLPRI;
  }
  if (events & Channel::kWriteEvent)
  {
    pevents |= POLLOUT;
  }
  int index = channel->index();

  LOG_TRACE << "index=" << index << " fd=" << fd << " events=" << events << " pevents=" << pevents;

  if (index < 0)
  {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = pevents;
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    channel->setIndex(index_++);
    channels_[fd] = channel;
  }
  else //update
  {
    pollfds_[index].events = pevents;
    pollfds_[index].revents = 0;
  }
};

void PollSelector::removeChannel(Channel *channel)
{
  int index = channel->index();
  int fd = channel->fd();
  channels_.erase(fd);

  if (index == pollfds_.size() - 1)
  {
    pollfds_.pop_back();
  }
  else
  {
    std::swap(pollfds_[index], pollfds_[pollfds_.size() - 1]);
    pollfds_.pop_back();
    int newfd = pollfds_[index].fd;
    channels_[newfd]->setIndex(index);
  }
  channel->setIndex(-1);
  index_--;
};

void PollSelector::selectNow(ChannelList& activeChannles)
{
  select(0, activeChannles);
};

void PollSelector::select(ChannelList& activeChannles)
{
  select(-1, activeChannles);
};

void PollSelector::select(int timeoutMs, ChannelList& activeChannles)
{
  
  int num = ::poll(&(*(pollfds_.begin())), pollfds_.size(), timeoutMs);
  LOG_TRACE << "timeout = " << timeoutMs  << " num = " << num;
  //poll() returns the number of descriptors that are ready for I/O, or -1 if an error occurred.  
  if (num < 0)
  {
    LOG_ERROR << "timeout = " << timeoutMs << " error = " << log::Error();
    return;
  }
  if (num == 0)
  {
    return;
  }

  for (PollFds::iterator itr = pollfds_.begin(); itr != pollfds_.end() && num > 0; itr++)
  {

    int fd = itr->fd;
    int events = itr->revents;
    if (events <= 0) continue;
    int revents = 0;
    if (events & (POLLNVAL | POLLERR | POLLHUP))
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
    LOG_TRACE << "fd = " << fd << " revents = " << revents << " poll_revents = " << events;
    Channel* channel = channels_[fd];
    channel->setRevents(revents);
    activeChannles.push_back(channel);
    num--;
  }
};

}
}