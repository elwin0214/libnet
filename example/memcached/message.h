#ifndef __LIBNET_MEMCACHED_MESSAGE_H__
#define __LIBNET_MEMCACHED_MESSAGE_H__

#include <libnet/buffer.h>
#include <libnet/nocopyable.h>
#include <libnet/countdown_latch.h>
#include <string>
#include <memory>
#include "command.h"
#include "future.h"

class Message : public NoCopyable
{
public:
  Message(Command* command, const std::shared_ptr<Future>& future)
    : command_(command),
      future_(future)
  {

  };

  // Code code()
  // {
  //   return command_->code();
  // };

  // std::string desc()
  // {
  //   return command_->desc();
  // };

  // std::string result()
  // {
  //   return command_->result();
  // };

  void append(Buffer& buffer)
  {
    command_->append(buffer);
  };

  bool parse(Buffer& buffer)
  {
    return command_->parse(buffer);
  };

  // void wait()
  // {
  //   future_.wait();
  // };

  void wakeup()
  {
    future_->set(command_->code(), command_->result());
    future_->wakeup();
  }


private:
  //CountDownLatch countDownLatch_;
  std::unique_ptr<Command> command_;
  std::shared_ptr<Future> future_;
};

#endif