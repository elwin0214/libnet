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
    pevents |= POLLIN;
  }
  if (events & Channel::kWriteEvent)
  {
    pevents |= POLLOUT;
  }
  int index = channel->index();
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
  if (num < 0)
  {
    LOG_ERROR << "timeout = " <<timeoutMs ;
    return;
  }
  if (num == 0)
  {
    LOG_DEBUG << "timeout = " << timeoutMs;
    return;
  }

  for (PollFds::iterator itr = pollfds_.begin(); itr != pollfds_.end(); itr++)
  {
    int fd = itr->fd;
    int events = itr->revents;
    int revents = 0;
    if (events & POLLIN)
    {
      revents |= Channel::kReadEvent;
    }
    if (events & POLLOUT)
    {
      revents |= Channel::kWriteEvent;
    }
    Channel* channel = channels_[fd];
    channel->setRevents(revents);
    activeChannles.push_back(channel);
  }
};

}
}