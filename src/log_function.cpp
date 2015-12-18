#include "log_function.h"

namespace linear {

namespace log {

LogFunction::~LogFunction() {
  Disable();
}

bool LogFunction::Available() {
  linear::lock_guard<linear::mutex> lock(mutex_);
  return (callback_ != NULL);
}

bool LogFunction::Enable(LogCallback callback) {
  linear::lock_guard<linear::mutex> lock(mutex_);
  callback_ = callback;
  return true;
}

void LogFunction::Disable() {
  linear::lock_guard<linear::mutex> lock(mutex_);
  callback_ = NULL;
}

void LogFunction::Write(bool debug, Level level, const char* file, int line, const char* func, const char* message) {
  (void) debug; // not used debug flag here now
  linear::lock_guard<linear::mutex> lock(mutex_);
  if (callback_ == NULL) {
    return;
  }
 (*callback_)(level, file, line, func, message);
}

}  // namespace log

}  // namespace linear
