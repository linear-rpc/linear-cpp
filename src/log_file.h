#ifndef	LINEAR_LOG_FILE_H_
#define	LINEAR_LOG_FILE_H_

#include <stdio.h>

#include "log.h"

namespace linear {

namespace log {

class LogFile : public Log {
 public:
  LogFile() : fp_(NULL), color_(false) {}
  ~LogFile();
  virtual bool Available();
  virtual bool Enable(const std::string& filename);
  virtual void Disable();
  void Colorize(bool flag);
  void Write(bool debug, linear::log::Level level, const char* file, int line, const char* func, const char* message);

 protected:
  FILE* fp_;
  bool color_;

 private:
  LogFile(const LogFile& rhs);
  LogFile& operator=(const LogFile& rhs);
};

}  // namespace log

}  // namespace linear

#endif	// LINEAR_LOG_FILE_H_
