#ifndef	LINEAR_LOG_INTERNAL_H_
#define	LINEAR_LOG_INTERNAL_H_

#include "linear/log.h"
#include "linear/mutex.h"

#define LOG_BUFSIZ (16384)

namespace linear {

namespace log {

class Log {
 public:
  virtual void Write(bool debug, Level level, const char* fname, int line, const char* func, const char* message) = 0;

 protected:
  Log() {}
  Log(const Log& rhs);
  Log& operator=(const Log& rhs);
  virtual ~Log() {}
  virtual bool Available() = 0;
  std::string GetDateTime();

  linear::mutex mutex_;
};

}  // namespace log

}  // namespace linear

#endif	// LINEAR_LOG_INTERNAL_H_
