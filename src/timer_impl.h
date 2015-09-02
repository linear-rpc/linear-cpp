#ifndef LINEAR_TIMER_IMPL_H_
#define LINEAR_TIMER_IMPL_H_

#include "tv.h"

#include "linear/error.h"
#include "linear/mutex.h"
#include "linear/timer.h"

namespace linear {

class TimerImpl {
 public:
  enum State {
    START,
    STOP
  };

  TimerImpl();
  ~TimerImpl();

  int GetId();
  linear::Error Start(int id, TimerCallback callback, unsigned int timeout, void* args);
  void Stop();
  void OnTimer();

 private:
  State state_;
  tv_timer_t* tv_timer_;
  linear::TimerCallback callback_;
  void* args_;
  linear::mutex mutex_;
};

} // namespace linear

#endif // LINEAR_TIMER_IMPL_
