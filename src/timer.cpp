#include "linear/timer.h"

#include "timer_impl.h"
#include "linear/log.h"

using namespace linear::log;

namespace linear {

Timer::Timer() : timer_(new TimerImpl()) {
}

Timer::Timer(const Timer& timer) : timer_(timer.timer_) {
}

Timer& Timer::operator=(const Timer& timer) {
  timer_ = timer.timer_;
  return *this;
}

Timer::~Timer() {
}

int Timer::GetId() const {
  if (!timer_) {
    return -1;
  }
  return timer_->GetId();
}

Error Timer::Start(linear::TimerCallback callback, unsigned int timeout, void* args) const {
  if (!timer_) {
    return Error(LNR_EINVAL);
  }
  Error e(LNR_ENOMEM);
  try {
    EventLoop::TimerEvent* ev = new EventLoop::TimerEvent(timer_);
    e = timer_->Start(callback, timeout, args, ev);
    if (e != Error(LNR_OK)) {
      delete ev;
    }
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
  }
  return e;
}

void Timer::Stop() const {
  if (timer_) {
    timer_->Stop();
  }
}

}  // namespace linear
