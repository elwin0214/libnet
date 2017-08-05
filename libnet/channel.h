#ifndef __LIBNET_CHANNEL_H__
#define __LIBNET_CHANNEL_H__

#include <libnet/nocopyable.h>
#include <memory>
#include <functional>

namespace libnet
{

class EventLoop;

class Channel : public NoCopyable 
{
public:
  typedef std::function<void()>  EventCallBack;

  static const int kReadEvent;
  static const int kWriteEvent;
  static const int kNoneEvent;
  static const int kErrorEvent;

public:
  Channel(EventLoop *loop, int fd);
  int index() { return index_; }
  void setIndex(int index) { index_ = index; }
  int events() { return events_; }
  int revents(){ return revents_; }
  void setRevents(int revents) {revents_ = revents; }
  int fd() {return fd_; }

  void disableAll() {events_ = kNoneEvent; update();}
  void remove();
  void enableReading() { events_ |= kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }
  void disableReading() { events_ &= ~kReadEvent; update(); }
  void disableWriting() { events_ &= ~kWriteEvent; update(); }

  bool isWriting() {return events_ & kWriteEvent;}
  void handleEvent();
         
  void setReadCallback(EventCallBack callback) {readCallBack_ = callback; }
  void setWriteCallback(EventCallBack callback) {writeCallBack_ = callback; }
  void setCloseCallback(EventCallBack callback) {closeCallBack_ = callback; }
  void setErrorCallback(EventCallBack callback) {errorCallBack_ = callback; }

private:
  void update();

private:
  int fd_;
  int index_;
  int events_;
  int revents_;

  EventCallBack readCallBack_;
  EventCallBack writeCallBack_;
  EventCallBack closeCallBack_;
  EventCallBack errorCallBack_;
  EventLoop *loop_;

};

}

#endif