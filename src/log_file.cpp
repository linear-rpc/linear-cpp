#include "log_file.h"

namespace linear {

namespace log {

LogFile::~LogFile() {
  Disable();
}

bool LogFile::Available() {
  linear::lock_guard<linear::mutex> lock(mutex_);
  return (fp_ != NULL);
}

bool LogFile::Enable(const std::string& filename) {
  linear::lock_guard<linear::mutex> lock(mutex_);

#ifdef _WIN32
  fp_ = _fsopen(filename.c_str(), "w", _SH_DENYWR);
#else
  fp_ = fopen(filename.c_str(), "w");
#endif

  if (fp_ == NULL) {
    return false;
  }
  return true;
}

void LogFile::Disable() {
  linear::lock_guard<linear::mutex> lock(mutex_);
  if (fp_ != NULL) {
    fclose(fp_);
    fp_ = NULL;
  }
}

void LogFile::Colorize(bool flag) {
  color_ = flag;
}

void LogFile::Write(bool debug, Level level, const char* file, int line, const char* func, const char* message) {
  linear::lock_guard<linear::mutex> lock(mutex_);

  if (fp_ == NULL) {
    return;
  }

#define LINEAR_LOG_LEVEL_CASE_GEN(IDENT, NUM, LONG_STR, SHORT_STR, COLOR)     \
  case IDENT:                                                                 \
    strptr = SHORT_STR;                                                       \
    col = COLOR;                                                              \
  break;

  const char* strptr = "";
  int col = COLOR_DEFAULT;
  switch(level) {
    LINEAR_LOG_LEVEL_MAP(LINEAR_LOG_LEVEL_CASE_GEN)
  default:
      break;
  }
#undef LINEAR_LOG_LEVEL_CASE_GEN

  if (color_) {
    if (debug) {
      fprintf(fp_, "\x1b[7m");
    }
    fprintf(fp_, "\x1b[%dm", col);
  }
  
#ifdef _LINEAR_LOG_DEBUG_
  std::string fname(file);

# ifdef _WIN32
#  define SEPARATOR '\\'
# else
#  define SEPARATOR '/'
# endif
  std::string::size_type n = fname.find_last_of(SEPARATOR);
  (void) n;
# undef SEPARATOR

  fprintf(fp_, "%s: [%s] (%s:%d) %s\n",
          GetDateTime().c_str(),
          strptr,
          (n == std::string::npos) ? fname.c_str() : fname.substr(n + 1).c_str(), line,
          message);
#else
  (void)(file);
  (void)(line);
  (void)(func);
  fprintf(fp_, "%s: [%s] %s\n", GetDateTime().c_str(), strptr, message);
#endif

  if (color_) {
    fprintf(fp_, "\x1b[%dm", COLOR_DEFAULT);
    if (debug) {
      fprintf(fp_, "\x1b[0m");
    }
  }
  fflush(fp_);
}

}  // namespace log

}  // namespace linear
