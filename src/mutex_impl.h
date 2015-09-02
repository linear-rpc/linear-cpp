#ifndef LINEAR_MUTEX_IMPL_H_
#define LINEAR_MUTEX_IMPL_H_

#include "linear/mutex.h"

#include "uv.h"

namespace linear {

class mutex::mutex_impl {
 public:
  typedef uv_mutex_t* native_handle_type;

  mutex_impl();
  ~mutex_impl();

  void lock();
  bool try_lock();
  void unlock();

  uv_mutex_t* native_handle();

 private:
  uv_mutex_t mutex_;
};

}  // namespace linear

#endif // LINEAR_MUTEX_IMPL_H_
