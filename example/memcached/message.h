#ifndef __LIBNET_MEMCACHED_MESSAGE_H__
#define __LIBNET_MEMCACHED_MESSAGE_H__

#include <libnet/buffer.h>
#include <libnet/nocopyable.h>
#include <libnet/countdown_latch.h>
#include <string>
#include <memory>
#include "command.h"

class Message : public NoCopyable
{
public:
  Message(Command* command)
    : countDownLatch_(1),
      command_(command)
  {

  };

  Code code()
  {
    return command_->code();
  };

  std::string result()
  {
    return command_->result();
  };

  void append(Buffer& buffer)
  {
    command_->append(buffer);
  };

  void parse(Buffer& buffer)
  {
    command_->parse(buffer);
  };

  void wait()
  {
    countDownLatch_.wait();
  };

  void wakeup()
  {
    countDownLatch_.countDown();
  }

private:
  CountDownLatch countDownLatch_;
  std::unique_ptr<Command> command_;
};

#endif