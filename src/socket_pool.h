#ifndef LINEAR_SOCKET_POOL_H_
#define LINEAR_SOCKET_POOL_H_

#include <assert.h>

#include <list>

#include "linear/log.h"
#include "linear/mutex.h"
#include "linear/group.h"

#include "socket_impl.h"

namespace linear {

class SocketPool {
 public:
  SocketPool() : max_(-1) {
  }
  ~SocketPool() {}
  void SetMaxLimit(size_t max) {
    max_ = max;
  }
  linear::Error Add(const linear::shared_ptr<linear::SocketImpl>& s) {
    int id = s->GetId();
    if (id < 0) {
      return Error(LNR_EINVAL);
    }
    linear::lock_guard<linear::mutex> lock(mutex_);
    if (max_ > 0 && max_ <= pool_.size()) {
      LINEAR_LOG(linear::log::LOG_WARN, "Socket(type = %d, id = %d) excess MaxLimits(%d)",
                 s->GetType(), id, max_);
      return Error(LNR_ENOSPC);
    }
    for (std::vector<linear::shared_ptr<linear::SocketImpl> >::iterator it = pool_.begin();
         it != pool_.end(); it++) {
      if (id == (*it)->GetId()) {
        LINEAR_LOG(linear::log::LOG_WARN, "Socket(type = %d, id = %d) already exists", s->GetType(), id);
        return Error(LNR_OK);
      }
    }
    pool_.push_back(s);
    LINEAR_DEBUG(linear::log::LOG_DEBUG, "Socket(type = %d, id = %d) is added", s->GetType(), id);
    return Error(LNR_OK);
  }
  void Remove(const linear::shared_ptr<linear::SocketImpl>& s) {
    int id = s->GetId();
    if (id < 0) {
      return;
    }
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::vector<linear::shared_ptr<linear::SocketImpl> >::iterator it = pool_.begin();
         it != pool_.end(); it++) {
      if (id == (*it)->GetId()) {
        LINEAR_DEBUG(linear::log::LOG_DEBUG, "Socket(type = %d, id = %d) is removed", s->GetType(), id);
        pool_.erase(it);
        return;
      }
    }
    LINEAR_LOG(linear::log::LOG_WARN, "Socket(id = %d) is already removed", id);
  }
  void Clear() {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::vector<linear::shared_ptr<linear::SocketImpl> >::iterator it = pool_.begin();
         it != pool_.end(); it++) {
      linear::Group::LeaveAll(Socket(*it));
    }
    pool_.clear();
  }

 protected:
  size_t max_;
  std::vector<shared_ptr<linear::SocketImpl> > pool_;
  linear::mutex mutex_;
};

} // namespace linear

#endif // LINEAR_SOCKET_POOL_H_
