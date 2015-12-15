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

/**
 * @class EventLoop event_loop.h "linear/event_loop.h"
 * EventLoop class.
 * enable to use specific event loop(a.k.a thread) at clients and servers
 * @see linear::TCPServer, linear::TCPClient etc.
 */
class LINEAR_EXTERN EventLoop {
 public:
  static const EventLoop& GetDefault();

 public:
  /// @cond hidden
  EventLoop();
  ~EventLoop();
  const linear::shared_ptr<linear::EventLoopImpl> GetImpl() const;
  /// @endcond

 private:
  linear::shared_ptr<linear::EventLoopImpl> loop_;
};

}  // namespace linear

#endif  // LINEAR_EVENT_LOOP_H_
