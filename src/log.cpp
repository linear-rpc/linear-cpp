#include <stdarg.h>
#include <time.h>

#include "linear/memory.h"

#include "log_stderr.h"
#include "log_function.h"

#ifdef _WIN32
# include <stdlib.h>
# include <windows.h>
#else
# include <sys/time.h>
#endif

namespace linear {

namespace log {

static Level g_level = LOG_OFF;

// ref(but there's bit differences):
// http://stackoverflow.com/questions/16140294/is-it-right-way-to-create-sinlgeton-class-by-weak-ptr
static linear::shared_ptr<LogStderr> g_stderr = linear::shared_ptr<LogStderr>(new LogStderr());
static linear::shared_ptr<LogFile> g_file = linear::shared_ptr<LogFile>(new LogFile());
static linear::shared_ptr<LogFunction> g_function = linear::shared_ptr<LogFunction>(new LogFunction());

static linear::weak_ptr<LogStderr>& GetLogStderr() {
  static linear::weak_ptr<LogStderr> weak = g_stderr;
  return weak;
}
static linear::weak_ptr<LogFile>& GetLogFile() {
  static linear::weak_ptr<LogFile> weak = g_file;
  return weak;
}
static linear::weak_ptr<LogFunction>& GetLogFunction() {
  static linear::weak_ptr<LogFunction> weak = g_function;
  return weak;
}

/* functions */
Level GetLevel() {
  return g_level;
}

void SetLevel(Level level) {
  g_level = level;
}

bool EnableStderr() {
  if (linear::shared_ptr<LogStderr> shared = GetLogStderr().lock()) {
    return shared->Enable();
  }
  return false;
}

bool EnableFile(const std::string& filename) {
  if (linear::shared_ptr<LogFile> shared = GetLogFile().lock()) {
    return shared->Enable(filename);
  }
  return false;
}

bool EnableCallback(LogCallback callback) {
  if (linear::shared_ptr<LogFunction> shared = GetLogFunction().lock()) {
    return shared->Enable(callback);
  }
  return false;
}

void DisableStderr() {
  if (linear::shared_ptr<LogStderr> shared = GetLogStderr().lock()) {
    shared->Disable();
  }
}

void DisableFile() {
  if (linear::shared_ptr<LogFile> shared = GetLogFile().lock()) {
    shared->Disable();
  }
}

void DisableCallback() {
  if (linear::shared_ptr<LogFunction> shared = GetLogFunction().lock()) {
    shared->Disable();
  }
}

void Colorize(bool flag) {
  if (linear::shared_ptr<LogStderr> shared = GetLogStderr().lock()) {
    shared->Colorize(flag);
  }
}

bool DoPrint(linear::log::Level level) {
  return (level <= g_level);
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

  if (linear::shared_ptr<LogStderr> shared = GetLogStderr().lock()) {
    shared->Write(debug, level, file, line, func, buffer);
  }
  if (linear::shared_ptr<LogFile> shared = GetLogFile().lock()) {
    shared->Write(debug, level, file, line, func, buffer);
  }
  if (linear::shared_ptr<LogFunction> shared = GetLogFunction().lock()) {
    shared->Write(debug, level, file, line, func, buffer);
  }
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
