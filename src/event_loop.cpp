#include "linear/event_loop.h"

#include "event_loop_impl.h"

namespace linear {

// -fthreadsafe-statics
const EventLoop& EventLoop::GetDefault() {
  static EventLoop g_loop;
  return g_loop;
}

EventLoop::EventLoop() : loop_(new EventLoopImpl()) {
}

EventLoop::~EventLoop() {
}  

const linear::shared_ptr<linear::EventLoopImpl> EventLoop::GetImpl() const {
  return loop_;
}

} // namespace linear
