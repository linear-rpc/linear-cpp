#ifndef	LINEAR_LOG_FUNCTION_H_
#define	LINEAR_LOG_FUNCTION_H_

#include "log.h"

namespace linear {

namespace log {

class LogFunction : public Log {
 public:
  LogFunction() : callback_(NULL) {}
  ~LogFunction() {}
  bool Available();
  bool Enable(LogCallback callback);
  void Disable();
  void Write(bool debug, linear::log::Level level, const char* file, int line, const char* func, const char* message);

 private:
  LogFunction(const LogFunction& rhs);
  LogFunction& operator=(const LogFunction& rhs);
  LogCallback callback_;
};

}  // namespace log

}  // namespace linear

#endif	// LINEAR_LOG_FUNCTION_H_
