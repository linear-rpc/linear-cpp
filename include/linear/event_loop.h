/**
 * @file event_loop.h
 * Event loop class definition
 */

#ifndef LINEAR_EVENT_LOOP_H_
#define LINEAR_EVENT_LOOP_H_

#include "linear/memory.h"
#include "linear/private/extern.h"

namespace linear {

class EventLoopImpl;

class LINEAR_EXTERN EventLoop {
 public:
  static const EventLoop& GetDefault();

 public:
  EventLoop();
  ~EventLoop();
  const linear::shared_ptr<linear::EventLoopImpl> GetImpl() const;

 private:
  linear::shared_ptr<linear::EventLoopImpl> loop_;
};

}  // namespace linear

#endif  // LINEAR_EVENT_LOOP_H_
