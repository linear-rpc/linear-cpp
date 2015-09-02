#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "log_stderr.h"
#include "log_file.h"
#include "log_function.h"
#include "linear/mutex.h"

#ifdef _WIN32
# include <stdlib.h>
# include <windows.h>
#else
# include <sys/time.h>
#endif

namespace linear {

namespace log {

static Level g_level = LOG_OFF;

static LogStderr& GetLogStderr() {
  static LogStderr s_stderr;
  return s_stderr;
}
static LogFile& GetLogFile() {
  static LogFile s_file;
  return s_file;
}
static LogFunction& GetLogFunction() {
  static LogFunction s_function;
  return s_function;
}

/* functions */
Level GetLevel() {
  return g_level;
}

void SetLevel(Level level) {
  g_level = level;
}

bool EnableStderr() {
  return GetLogStderr().Enable();
}

bool EnableFile(const std::string& filename) {
  return GetLogFile().Enable(filename);
}

bool EnableCallback(LogCallback callback) {
  return GetLogFunction().Enable(callback);
}

void DisableStderr() {
  GetLogStderr().Disable();
}

void DisableFile() {
  GetLogFile().Disable();
}

void DisableCallback() {
  GetLogFunction().Disable();
}

void Colorize(bool flag) {
  GetLogStderr().Colorize(flag);
}

bool DoPrint(linear::log::Level level) {
  return ((GetLogStderr().Available() || GetLogFile().Available() || GetLogFunction().Available()) && (level <= g_level));
}

void Print(bool debug, linear::log::Level level, const char* file, int line, const char* func, const char* format, ...) {
  va_list args;
  char buffer[LOG_BUFSIZ];

  va_start(args, format);
#ifdef _WIN32
  vsnprintf_s(buffer, LOG_BUFSIZ, _TRUNCATE, format, args);
#else
  vsnprintf(buffer, LOG_BUFSIZ, format, args);
#endif
  va_end(args);

  GetLogStderr().Write(debug, level, file, line, func, buffer);
  GetLogFile().Write(debug, level, file, line, func, buffer);
  GetLogFunction().Write(debug, level, file, line, func, buffer);
}

/* Log class methods */
std::string Log::GetDateTime() {
  char datetime_str[32];

#ifdef _WIN32
  SYSTEMTIME st;
  GetLocalTime(&st);
  _snprintf_s(datetime_str, sizeof(datetime_str), _TRUNCATE,
              "%d-%02d-%02d %02d:%02d:%02d.%03d",
              st.wYear, st.wMonth, st.wDay,
              st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
  struct tm ts;
  struct timeval now;
  struct tm* ret = NULL;
  if (gettimeofday(&now, 0) == 0) {
    ret = localtime_r(&now.tv_sec, &ts);
  }
  if (ret == NULL) {
    snprintf(datetime_str, sizeof(datetime_str), "ERR: fail to get date");
  } else {
    snprintf(datetime_str, sizeof(datetime_str),
             "%d-%02d-%02d %02d:%02d:%02d.%03d",
             ts.tm_year + 1900, ts.tm_mon + 1, ts.tm_mday,
             ts.tm_hour, ts.tm_min, ts.tm_sec,
             static_cast<int>(now.tv_usec / 1000));
  }
#endif

  return std::string(datetime_str);
}

}  // namespace log

}  // namespace linear
