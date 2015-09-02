#ifndef LINEAR_NONCE_POOL_H_
#define LINEAR_NONCE_POOL_H_

#include <string>
#include <vector>

#include "linear/log.h"
#include "linear/mutex.h"
#include "linear/timer.h"

#define NONCE_TIMEOUT (60000) // 1 min

namespace linear {

class NoncePool {
 public:
  struct TimerCtx {
    TimerCtx(const std::string& n, NoncePool* p) : nonce(n), pool(p) {}
    ~TimerCtx() {}
    std::string nonce;
    NoncePool* pool;
  };
  struct Nonce {
    Nonce() {}
    Nonce(const std::string& n, TimerCtx* c) : nonce(n), ctx(c) {}
    ~Nonce() {}
    std::string nonce;
    linear::Timer timer;
    TimerCtx* ctx;
  };

  static void OnTimer(void* args) {
    NoncePool::TimerCtx* ctx = reinterpret_cast<NoncePool::TimerCtx*>(args);
    ctx->pool->Remove(ctx->nonce);
  }

 public:
  NoncePool() {}
  ~NoncePool() {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::vector<NoncePool::Nonce>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      it->timer.Stop();
      delete it->ctx;
      pool_.erase(it);
      return;
    }
  }
  linear::Error Add(const std::string& nonce, int timeout = NONCE_TIMEOUT) {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::vector<NoncePool::Nonce>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (nonce == it->nonce) {
        return Error(LNR_EALREADY);
      }
    }
    TimerCtx* c = new TimerCtx(nonce, this);
    Nonce n(nonce, c);
    pool_.push_back(n);
    LINEAR_DEBUG(linear::log::LOG_DEBUG, "Nonce(%s...) is valid for %d msecs", nonce.substr(16).c_str(), timeout);
    return n.timer.Start(NoncePool::OnTimer, timeout, c);
  }
  void Remove(const std::string& nonce) {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::vector<NoncePool::Nonce>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (nonce == it->nonce) {
        LINEAR_DEBUG(linear::log::LOG_DEBUG, "Nonce(%s...) is removed", nonce.substr(16).c_str());
        it->timer.Stop();
        delete it->ctx;
        pool_.erase(it);
        return;
      }
    }
  }
  bool IsValid(const std::string& nonce) {
    linear::lock_guard<linear::mutex> lock(mutex_);
    for (std::vector<NoncePool::Nonce>::iterator it = pool_.begin(); it != pool_.end(); it++) {
      if (nonce == it->nonce) {
        LINEAR_DEBUG(linear::log::LOG_DEBUG, "Nonce(%s...) is valid", nonce.substr(16).c_str());
        return true;
      }
    }
    LINEAR_DEBUG(linear::log::LOG_WARN, "Nonce(%s...) is invalid", nonce.substr(16).c_str());
    return false;
  }

 protected:
  std::vector<Nonce> pool_;
  linear::mutex mutex_;
};

} // namespace linear

#endif // LINEAR_NONCE_POOL_H_
