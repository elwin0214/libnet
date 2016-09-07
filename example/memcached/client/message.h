#ifndef __LIBNET_MEMCACHED_CLIENT_MESSAGE_H__
#define __LIBNET_MEMCACHED_CLIENT_MESSAGE_H__

#include <libnet/buffer.h>
#include <libnet/nocopyable.h>
#include <libnet/countdown_latch.h>
#include <string>
#include <memory>
#include "command.h"
#include "future.h"
namespace memcached
{
namespace client
{

struct Caller
{
  virtual void append(Buffer& buffer) = 0;
  virtual bool parse(Buffer& buffer) = 0; 
  virtual void wakeup() = 0;
};

template<typename T>
class Message : public NoCopyable, public Caller
{
public:
  Message(Command<T>* command, const std::shared_ptr<Future<T>>& future)
    : command_(command),
      future_(future)
  {

  };

  void append(Buffer& buffer)
  {
    command_->append(buffer);
  };

  bool parse(Buffer& buffer)
  {
    return command_->parse(buffer);
  };

  void wakeup()
  {
    future_->set(command_->code(), command_->result());
    future_->wakeup();
  }


private:
  std::shared_ptr<Command<T>> command_;  //can not use unique_ptr??
  std::shared_ptr<Future<T>> future_;
};

}
}
#endif