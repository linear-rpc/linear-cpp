#ifndef LINEAR_SOCKET_POOL_H_
#define LINEAR_SOCKET_POOL_H_

#include <assert.h>

#include <list>

#include "linear/log.h"
#include "linear/mutex.h"
#include "linear/socket.h"
#include "linear/group.h"

namespace linear {

class SocketPool {
 public:
  SocketPool() : max_(-1), cur_(0) {
  }
  virtual ~SocketPool() {}
  virtual void SetMaxLimit(int max) {
    max_ = max;
  }
  virtual linear::Error Add(const Socket& socket) {
    int id = socket.GetId();
    if (id < 0) {
      return Error(LNR_OK);
    }
    linear::lock_guard<linear::mutex> lock(mutex_);
    if (max_ > 0 && max_ <= cur_) {
      LINEAR_LOG(linear::log::LOG_WARN, "Socket(type = %d, id = %d) excess MaxLimits(%d) <= current(%d)",
                 socket.GetType(), id, max_, cur_);
      return Error(LNR_ENOSPC);
    }
    for (std::list<linear::Socket>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (id == it->GetId()) {
        LINEAR_LOG(linear::log::LOG_WARN, "Socket(type = %d, id = %d) already exists", socket.GetType(), id);
        return Error(LNR_EALREADY);
      }
    }
    cur_++;
    pool_.push_back(socket);
    LINEAR_DEBUG(linear::log::LOG_DEBUG, "Socket(type = %d, id = %d) is added", socket.GetType(), id);
    return Error(LNR_OK);
  }
  virtual void Remove(int id) {
    if (id < 0) {
      return;
    }
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::list<linear::Socket>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (id == it->GetId()) {
        LINEAR_DEBUG(linear::log::LOG_DEBUG, "Socket(type = %d, id = %d) is removed", it->GetType(), id);
        cur_--;
        pool_.erase(it);
        return;
      }
    }
    LINEAR_LOG(linear::log::LOG_WARN, "Socket(id = %d) is already removed", id);
  }
  virtual const Socket& Get(int id) {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::list<Socket>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (id == it->GetId()) {
        LINEAR_DEBUG(linear::log::LOG_DEBUG, "found Socket(type = %d, id = %d)", it->GetType(), id);
        return (*it);
      }
    }
    LINEAR_LOG(linear::log::LOG_WARN, "not found Socket(id = %d), may be already removed", id);
    static Socket nil_sock;
    return nil_sock;
  }
  virtual void Clear() {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::list<Socket>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      linear::Group::LeaveAll(*it);
    }
    cur_ = 0;
    pool_.clear();
  }

 protected:
  int max_, cur_;
  std::list<linear::Socket> pool_;
  linear::mutex mutex_;
};

} // namespace linear

#endif // LINEAR_SOCKET_POOL_H_
