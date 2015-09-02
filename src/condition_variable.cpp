#include <cassert>

#include "linear/condition_variable.h"
#include "linear/mutex.h"

#include "mutex_impl.h"

#include "uv.h"

namespace linear {

// class condition_variable_impl
class condition_variable::condition_variable_impl {
 public:
  typedef uv_cond_t* native_handle_type;

  condition_variable_impl() {
    if (uv_cond_init(&cond_)) {
      assert(false);
    }
  }
  ~condition_variable_impl() {
    uv_cond_destroy(&cond_);
  }

  void notify_one() {
    uv_cond_signal(&cond_);
  }
  void notify_all() {
    uv_cond_broadcast(&cond_);
  }

  void wait(linear::unique_lock<linear::mutex>& lock) {
    uv_cond_wait(&cond_, lock.mutex()->native_handle()->native_handle());
  }

  native_handle_type native_handle() {
    return &cond_;
  }

 private:
  uv_cond_t cond_;
};

// class condition_variable
condition_variable::condition_variable()
  : impl_(new condition_variable::condition_variable_impl()) {
}
condition_variable::~condition_variable() {
  delete impl_;
}
void condition_variable::notify_one() {
  impl_->notify_one();
}
void condition_variable::notify_all() {
  impl_->notify_all();
}
void condition_variable::wait(linear::unique_lock<linear::mutex>& lock) {
  impl_->wait(lock);
}
condition_variable::native_handle_type condition_variable::native_handle() {
  return impl_;
}

}  // namespace linear
