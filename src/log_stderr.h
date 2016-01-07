#ifndef	LINEAR_LOG_STDERR_H_
#define	LINEAR_LOG_STDERR_H_

#include "log_file.h"

namespace linear {

namespace log {

class LogStderr : public LogFile {
 public:
  LogStderr() : LogFile() {}
  ~LogStderr();
  bool Enable(const std::string&);
  bool Enable();
  void Disable();

 private:
  LogStderr(const LogStderr& rhs);
  LogStderr& operator=(const LogStderr& rhs);
};

}  // namespace log

}  // namespace linear

#endif	// LINEAR_LOG_STDERR_H_
