/**
 * @file condition_variable.h
 * Condition variable class definition
 */

#ifndef LINEAR_CONDITION_VARIABLE_H_
#define LINEAR_CONDITION_VARIABLE_H_

#include "linear/private/extern.h"

#include "linear/mutex.h"

namespace linear {

class LINEAR_EXTERN condition_variable {
 public:
  class condition_variable_impl;
  typedef condition_variable_impl* native_handle_type;

  condition_variable();
  ~condition_variable();

  void notify_one();
  void notify_all();

  void wait(linear::unique_lock<linear::mutex>& lock);

  native_handle_type native_handle();

 private:
  condition_variable(const condition_variable&);
  condition_variable& operator=(const condition_variable&);

  condition_variable_impl* impl_;
};

}  // namespace linear
#endif  // LINEAR_CONDITION_VARIABLE_H_
