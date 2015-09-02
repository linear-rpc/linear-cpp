#ifndef LINEAR_TIMER_POOL_H_
#define LINEAR_TIMER_POOL_H_

#include <list>

#include "linear/log.h"
#include "linear/mutex.h"
#include "linear/timer.h"

using namespace linear::log;

namespace linear {

class TimerPool {
 public:
  TimerPool() {}
  ~TimerPool() {
    Clear();
  }
  void Add(const linear::Timer& timer) {
    int id = timer.GetId();
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::list<linear::Timer>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (id == it->GetId()) {
        LINEAR_LOG(LOG_WARN, "Timer(id = %d) already exists", id);
        return;
      }
    }
    pool_.push_back(timer);
    LINEAR_DEBUG(LOG_DEBUG, "Timer(id = %d) is added", id);
  }
  void Remove(int id) {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::list<linear::Timer>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (id == it->GetId()) {
        LINEAR_DEBUG(LOG_DEBUG, "Timer(id = %d) is removed", id);
        pool_.erase(it);
        return;
      }
    }
    LINEAR_LOG(LOG_ERR, "Timer(id = %d) is already removed", id);
  }
  void Clear() {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::list<linear::Timer>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      it->Stop();
    }
    pool_.clear();
  }
 protected:
  std::list<linear::Timer> pool_;
  linear::mutex mutex_;
};

} // namespace linear

#endif // LINEAR_TIMER_POOL_H_
