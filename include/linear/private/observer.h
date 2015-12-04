/**
 * @file observer.h
 * Instance observer class definition
 */

#ifndef LINEAR_PRIVATE_OBSERVER_H_
#define LINEAR_PRIVATE_OBSERVER_H_

#include "linear/mutex.h"

namespace linear {

template <class T>
class LINEAR_EXTERN Observer {
 public:
  explicit Observer(T* subject) : subject_(subject) {
  }
  ~Observer() {
    Terminate();
  }
  void Terminate() {
    Lock();
    subject_ = NULL;
    Unlock();
  }
  T* GetSubject() {
    return subject_;
  }
  void Lock() {
    mutex_.lock();
  }
  void Unlock() {
    mutex_.unlock();
  }
 private:
  T* subject_;
  linear::mutex mutex_;
};

} // namespace linear

#endif // LINEAR_PRIVATE_OBSERVER_H_
