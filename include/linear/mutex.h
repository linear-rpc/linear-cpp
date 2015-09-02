/**
 * @file mutex.h
 * Mutex and Lock class definition
 */

#ifndef LINEAR_MUTEX_H_
#define LINEAR_MUTEX_H_

#include "linear/private/extern.h"

namespace linear {

class LINEAR_EXTERN mutex {
 public:
  class mutex_impl;
  typedef mutex_impl* native_handle_type;

  mutex();
  ~mutex();

  void lock();
  bool try_lock();
  void unlock();

  native_handle_type native_handle();

 private:
  mutex(const mutex& m);
  mutex& operator=(const mutex& m);

  mutex_impl* impl_;
};

template <class Mutex>
class LINEAR_EXTERN lock_guard {
 public:
  typedef Mutex mutex_type;

  explicit lock_guard(mutex_type& m) : m_(m) {
    m_.lock();
  }
  ~lock_guard() {
    m_.unlock();
  }

 private:
  lock_guard(const lock_guard& lg);
  lock_guard& operator=(const lock_guard& lg);

  mutex_type& m_;
};

template <class Mutex>
class LINEAR_EXTERN unique_lock {
 public:
  typedef Mutex mutex_type;

  explicit unique_lock(mutex_type& m) : m_(m), owns_(false) {
    lock();
  }
  ~unique_lock() {
    if (owns_lock()) {
      m_.unlock();
    }
  }

  void lock() {
    if (owns_lock()) {
      return;  // TODO: assert(false) ?
    }
    m_.lock();
    owns_ = true;
  }
  bool try_lock() {
    return m_.try_lock();
  }
  void unlock() {
    if (!owns_lock()) {
      return;  // TODO: assert(false) ?
    }
    m_.unlock();
    owns_ = false;
  }

  mutex_type* mutex() const {
    return &m_;
  }
  bool owns_lock() const {
    return owns_;
  }

 private:
  unique_lock(const unique_lock& ul);
  unique_lock& operator=(const unique_lock& ul);

  mutex_type& m_;
  bool owns_;
};

} // namespace linear

#endif  // LINEAR_MUTEX_H_
