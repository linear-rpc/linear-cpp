#include <cassert>
#include <cstdlib>

#include "mutex_impl.h"

namespace linear {

// class mutex_impl
mutex::mutex_impl::mutex_impl() {
  if (uv_mutex_init(&mutex_)) {
    assert(false);
  }
}
mutex::mutex_impl::~mutex_impl() {
  uv_mutex_destroy(&mutex_);
}
void mutex::mutex_impl::lock() {
  uv_mutex_lock(&mutex_);
}
#define WAIT_UPDATING_LIBUV (1) // https://github.com/linear-rpc/linear-cpp/issues/1
bool mutex::mutex_impl::try_lock() {
#if WAIT_UPDATING_LIBUV && ! defined(_WIN32)
  int err;
  err = pthread_mutex_trylock(&mutex_);
  if (err) {
    if (err != EBUSY && err != EAGAIN && err != EDEADLK) {
      abort();
    }
    return false;
  }
  return true;
#else
  if (uv_mutex_trylock(&mutex_)) {
    return false;
  }
  return true;
#endif
}
#undef WAIT_UPDATING_LIBUV
void mutex::mutex_impl::unlock() {
  uv_mutex_unlock(&mutex_);
}
uv_mutex_t* mutex::mutex_impl::native_handle() {
  return &mutex_;
}

// class mutex
mutex::mutex()
  : impl_(new mutex::mutex_impl()) {
}
mutex::~mutex() {
  delete impl_;
}
void mutex::lock() {
  impl_->lock();
}
bool mutex::try_lock() {
  return impl_->try_lock();
}
void mutex::unlock() {
  impl_->unlock();
}
mutex::native_handle_type mutex::native_handle() {
  return impl_;
}

}  // namespace linear
