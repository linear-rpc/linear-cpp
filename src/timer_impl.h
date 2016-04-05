#ifndef LINEAR_TIMER_IMPL_H_
#define LINEAR_TIMER_IMPL_H_

#include "linear/mutex.h"
#include "linear/timer.h"

#include "event_loop_impl.h"

namespace linear {

class TimerImpl {
 public:
  enum State {
    STOP,
    START
  };

  TimerImpl(const linear::EventLoop& loop);
  TimerImpl(const linear::shared_ptr<linear::EventLoopImpl>& loop);
  ~TimerImpl();
  int GetId();
  linear::Error Start(TimerCallback callback, unsigned int timeout, void* args,
                      EventLoopImpl::TimerEvent* ev);
  void Stop();
  void OnTimer();

 private:
  int id_;
  linear::TimerImpl::State state_;
  linear::TimerCallback callback_;
  void* args_;
  tv_timer_t* tv_timer_;
  linear::mutex mutex_;
  linear::shared_ptr<linear::EventLoopImpl> loop_;
};

} // namespace linear

#endif // LINEAR_TIMER_IMPL_
