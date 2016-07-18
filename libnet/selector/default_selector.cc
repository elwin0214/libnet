#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include "../logger.h"
#include "../channel.h"
#include "default_selector.h"

namespace libnet
{
namespace selector
{

DefaultSelector::DefaultSelector(EventLoop *loop) : Selector(loop), max_fd_(0)//,index_(0)
    
{
  FD_ZERO(&read_fs_);
  FD_ZERO(&write_fs_);
  FD_ZERO(&error_fs_);
};

DefaultSelector::~DefaultSelector()
{

};

void DefaultSelector::selectNow(ChannelList& activeChannles)
{
  select(0, activeChannles);
};

void DefaultSelector::select(ChannelList& activeChannles)
{
  selectWithTimeval(NULL, activeChannles);
};

void DefaultSelector::select(int timeoutMs, ChannelList& activeChannles)
{
  LOG_DEBUG << "timeoutMs-" << timeoutMs;
  struct timeval *tvPtr = NULL;
  if (timeoutMs < 0) timeoutMs = 0;

  struct timeval tv;
  tv.tv_sec = timeoutMs / (1000);
  tv.tv_usec = timeoutMs % (1000) * 1000;
  tvPtr = &tv;
  selectWithTimeval(tvPtr, activeChannles);
};

void DefaultSelector::selectWithTimeval(struct timeval *tvPtr, ChannelList& activeChannles)
{
  fd_set read_fs;
  fd_set write_fs;
  fd_set error_fs;
    //FD_COPY(&read_fs_, &read_fs); //mac 
    //FD_COPY(&write_fs_, &write_fs);
  memcpy(static_cast<void*>(&read_fs), static_cast<void*>(&read_fs_), sizeof(fd_set));
  memcpy(static_cast<void*>(&write_fs), static_cast<void*>(&write_fs_), sizeof(fd_set));
  memcpy(static_cast<void*>(&error_fs), static_cast<void*>(&error_fs_), sizeof(fd_set));

  int numEvents = ::select(max_fd_ + 1, &read_fs, &write_fs, &error_fs, tvPtr);
  if (numEvents == 0)
  {
    LOG_DEBUG << "max_fd=" << max_fd_ << " ,numEvents-0";
    return;
  }
  else if (numEvents < 0)
  {
    LOG_SYSERROR << "max_fd=" << max_fd_ << " ,numEvents=" <<numEvents ;
    return;
  }

  for (std::map<int, Channel*>::iterator itr = channels_.begin(); itr != channels_.end(); itr++)// find event
  {
    Channel* channel = itr->second;
    int fd = itr->first;
    LOG_TRACE << "check event fd-" << fd ; 
    //bool has_event = false;
    int revents = 0;
    if (FD_ISSET(fd, &read_fs))
    {
      LOG_DEBUG << "poll readable, fd-" << fd;
      revents |= Channel::kReadEvent;
    }
    if (FD_ISSET(fd, &write_fs))
    {
      LOG_DEBUG << "poll writable, fd-" << fd;
      revents |= Channel::kWriteEvent;
    }
    if (FD_ISSET(fd, &error_fs))
    {
      LOG_DEBUG << "poll error, fd-" << fd;
      revents |= Channel::kErrorEvent;
    }
    if (revents > 0)
    {
      channel->setRevents(revents);
      activeChannles.push_back(channel);
    }
  }
};

void DefaultSelector::removeChannel(Channel *channel)
{
  
  int fd = channel->fd();
  LOG_DEBUG << "fd=" << fd;
  FD_CLR(fd, &read_fs_);
  FD_CLR(fd, &write_fs_);
  FD_CLR(fd, &error_fs_);
  channels_.erase(fd);
  if (channel->index() == 1)
  {
    updateChannel(Channel::kNoneEvent, channel);
  }
  //else if (channel->index() == 0)
  channel->setIndex(-1);
};

void DefaultSelector::updateChannel(int events, Channel *channel)
{
  int fd = channel->fd();
  LOG_DEBUG << "register event=" <<  events << "fd=" << fd;
  if (events & Channel::kReadEvent)
  {
    //LOG_DEBUG << "register read event, fd=" << fd;
    FD_SET(fd, &read_fs_);
  }
  else
  {
    //LOG_TRACE << "remove read event, fd=" << fd;
    FD_CLR(fd, &read_fs_);
  }
  if (events & Channel::kWriteEvent)
  {
    //LOG_DEBUG << "register write event, fd=" << fd;
    FD_SET(fd, &write_fs_);
  }
  else
  {
    //LOG_TRACE << "remove write event, fd=" << fd;
    FD_CLR(fd, &write_fs_);
  }
  if (events & Channel::kErrorEvent)
  {
    //LOG_DEBUG << "register error event, fd=" << fd;
    FD_SET(fd, &error_fs_);
  }
  else
  {
    //LOG_TRACE << "remove error event, fd=" << fd;
    FD_CLR(fd, &error_fs_);
  }
};

void DefaultSelector::updateChannel(Channel *channel)
{
  int fd = channel->fd();
  int events = channel->events();
  LOG_DEBUG << "fd=" << fd << " ,events=" << events;

  updateChannel(events, channel);

  if (channel->index() == -1)
  {
    max_fd_ = max_fd_ > fd ? max_fd_ : fd;
    LOG_DEBUG << "fd=" << fd << " ,max_fd=" << max_fd_;
    channels_.insert(std::map<int, Channel*>::value_type(fd, channel));
    //channel->setIndex(1);
  }
  else
  {
    if (events == Channel::kNoneEvent)
    {
      LOG_DEBUG << "fd=" << fd << " ,update no event" ;
    }
    //if (events == Channel::kNoneEvent)
     // LOG_WARN("update fd:%d with no event", fd);
  }

  if (events == Channel::kNoneEvent)
  {
    channel->setIndex(0);
  }
  else
  {
    channel->setIndex(1);
  }

};

}
}
