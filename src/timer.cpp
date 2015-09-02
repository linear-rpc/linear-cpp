#include "linear/timer.h"

#include "event_loop.h"
#include "timer_impl.h"

namespace linear {

static linear::mutex g_id_mutex;

static int Id() {
  lock_guard<mutex> lock(g_id_mutex);
  static int id = 0;
  return id++;
}

Timer::Timer() : id_(-1), timer_(new TimerImpl()) {
}

Timer::Timer(const Timer& timer) : id_(timer.id_), timer_(timer.timer_) {
}

Timer& Timer::operator=(const Timer& timer) {
  id_ = timer.id_;
  timer_ = timer.timer_;
  return *this;
}

Timer::~Timer() {
}

int Timer::GetId() const {
  return id_;
}

Error Timer::Start(linear::TimerCallback callback, unsigned int timeout, void* args) {
  int id = Id();
  linear::Error err = timer_->Start(id, callback, timeout, args);
  if (err.Code() == LNR_OK) {
    id_ = id;
    EventLoop::AddTimer(*this);
  }
  return err;
}

void Timer::Stop() {
  timer_->Stop();
  return;
}

}  // namespace linear
