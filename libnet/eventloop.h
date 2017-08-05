#ifndef __LIBNET_EVENTLOOP_H__
#define __LIBNET_EVENTLOOP_H__

#include <functional>
#include <queue>
#include <memory>
#include <atomic>
#include <libnet/mutexlock.h>
#include <libnet/timestamp.h>
#include <libnet/timer.h>
#include <libnet/timer_queue.h>
#include <libnet/current_thread.h>

namespace libnet
{
namespace selector
{
class Selector;
}
class Channel;

class EventLoop : public NoCopyable 
{

public:
  typedef std::function<void()>  Functor;

  EventLoop();

  ~EventLoop();

  void handleRead();

  void runInLoop(const Functor& functor);
  void queueInLoop(const Functor& functor);

  void runInLoop(Functor&& functor);
  void queueInLoop(Functor&& functor);

  void loop();

  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

  bool inLoopThread();
  void assertInLoopThread();

  void shutdown();
  void wakeup();

  TimerId runAt(const Timestamp &timestamp, const Functor& functor);
  TimerId runAfter(int delay, const Functor& functor);
  TimerId runInterval(int delay, int interval, const Functor& functor);

  TimerId runAt(const Timestamp &timestamp, Functor&& functor);
  TimerId runAfter(int delay, Functor&& functor);
  TimerId runInterval(int delay, int interval, Functor&& functor);

  void cancel(const TimerId& timerId) {timerQueue_->cancel(timerId); }
    
private:
    std::shared_ptr<selector::Selector> selector_;
    std::queue<Functor>  functors_;
    TID tid_;
    MutexLock lock_;
    std::atomic<bool> stop_;

    //std::atomic<bool> wakeup_;
    int wakeupFd_[2];
    Channel* wakeupChannel_;

    std::unique_ptr<TimerQueue> timerQueue_;

};

}

#endif