#include "log_stderr.h"

namespace linear {

namespace log {

LogStderr::~LogStderr() {
  Disable();
}

bool LogStderr::Enable(const std::string& filename) {
  return false;
}

bool LogStderr::Enable() {
  linear::lock_guard<linear::mutex> lock(mutex_);
  fp_ = stderr;
  return true;
}

void LogStderr::Disable() {
  linear::lock_guard<linear::mutex> lock(mutex_);
  fp_ = NULL;
}

}  // namespace log

}  // namespace linear
