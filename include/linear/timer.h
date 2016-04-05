/**
 * @file timer.h
 * Timer class definition
 **/

#ifndef LINEAR_TIMER_H_
#define LINEAR_TIMER_H_

#include "linear/error.h"
#include "linear/event_loop.h"
#include "linear/memory.h"

namespace linear {

/**
 * callback function definition for timer
 **/
typedef void (*TimerCallback)(void*);

class TimerImpl;

/**
 * @class Timer timer.h "linear/timer.h"
 * Oneshot timer class.
 */
class LINEAR_EXTERN Timer {
 public:
  /// @cond hidden
  Timer(const linear::EventLoop& loop = linear::EventLoop::GetDefault());
  Timer(const linear::shared_ptr<EventLoopImpl>& loop);
  virtual ~Timer();
  Timer(const Timer& timer);
  Timer& operator=(const Timer& timer);
  /// @endcond

  /**
   * Get Timer ID
   * @return timer id
   */
  int GetId() const;
  /**
   * Start timer with the specified parameter.
   * @param callback [in] callback function pointer called when timer event is fired
   * @param timeout [in] timeout time (ms)
   * @param args [in] callback arguments when callback function is called
   * @return linear::Error<br>
   * linear::LNR_OK on success, others on failure
   *
   @code
   static void OnOneshot(void* args) {
     int* count = reinterpret_cast<int*>(args);
     std::cout << "count: " << *count << std::endl;
     *count += 1;
     // if you want to continue oneshot timer, do like follows
     linear::OneshotTimer oneshot;
     oneshot.Start(OnTimer, 1000, count);
   }

   int main() {
     int count = 1;
     linear::Timer oneshot;
     oneshot.Start(OnOneshot, 1000, &count);
     std::string parameter;
     std::getline(std::cin, parameter);
   }
   @endcode
   */
  linear::Error Start(TimerCallback callback, unsigned int timeout, void* args) const;
  /**
   * Stop timer.
   */
  void Stop() const;

 private:
  shared_ptr<TimerImpl> timer_;
};

}  // namespace linear

#endif  // LINEAR_TIMER_H_
