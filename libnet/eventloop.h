#ifndef __LIBNET_EVENTLOOP_H__
#define __LIBNET_EVENTLOOP_H__

#include <functional>
#include <queue>
#include <memory>
#include <atomic>
#include "mutexlock.h"
#include "timestamp.h"
#include "timer.h"
#include "timer_queue.h"
#include "current_thread.h"

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
  TimerId runAfter(int afterTimeMs, const Functor& functor);
  TimerId runInterval(int afterTimeMs, int intervalMs, const Functor& functor);

  TimerId runAt(const Timestamp &timestamp, Functor&& functor);
  TimerId runAfter(int afterTimeMs, Functor&& functor);
  TimerId runInterval(int afterTimeMs, int intervalMs, Functor&& functor);

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