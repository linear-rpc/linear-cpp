/**
 * @file log.h
 * Log class definition
 **/

#ifndef LINEAR_LOG_H_
#define LINEAR_LOG_H_

#include <string>

#include "linear/private/extern.h"

/// @cond hidden
#ifdef _WIN32
# define __LINEAR_PRETTY_FUNCTION__ __FUNCTION__
#else
# define __LINEAR_PRETTY_FUNCTION__ __PRETTY_FUNCTION__
#endif

#define LINEAR_LOG(level, format, ...)                                  \
  if (linear::log::DoPrint(level)) {                                    \
    linear::log::Print(false, level, __FILE__, __LINE__, __LINEAR_PRETTY_FUNCTION__, format, ##__VA_ARGS__); \
  }

#ifdef _LINEAR_LOG_DEBUG_
#define LINEAR_DEBUG(level, format, ...)                                \
  if (linear::log::DoPrint(level)) {                                    \
    linear::log::Print(true, level, __FILE__, __LINE__, __LINEAR_PRETTY_FUNCTION__, format, ##__VA_ARGS__); \
  }
#else
# define LINEAR_DEBUG(level, format, ...)
#endif

#define LINEAR_LOG_PRINTABLE_STRING(any) \
  ((linear::log::GetLevel() == linear::log::LOG_FULL) ? any.stringify() : any.stringify(64, true))
/// @endcond

namespace linear {

/**
 * @namespace linear::log
 * namespace for log functions
 **/
namespace log {

/* LOG level Generator */
#define COLOR_BLACK   (30)
#define COLOR_RED     (31)
#define COLOR_GREEN   (32)
#define COLOR_YELLOW  (33)
#define COLOR_BLUE    (34)
#define COLOR_MAGENTA (35)
#define COLOR_CYAN    (36)
#define COLOR_WHITE   (37)
#define COLOR_DEFAULT (39)

#define LINEAR_LOG_LEVEL_MAP(GEN)                            \
  GEN(LOG_OFF,  -1, "",        ""   , COLOR_DEFAULT)         \
  GEN(LOG_ERR,   0, "ERROR",   "ERR", COLOR_RED)             \
  GEN(LOG_WARN,  1, "WARNING", "WRN", COLOR_MAGENTA)         \
  GEN(LOG_INFO,  2, "INFO",    "INF", COLOR_CYAN)            \
  GEN(LOG_DEBUG, 3, "DEBUG",   "DBG", COLOR_DEFAULT)         \
  GEN(LOG_FULL,  4, "DEBUG",   "DBG", COLOR_DEFAULT)

#define LINEAR_LOG_LEVEL_GEN(IDENT, NUM, LONG_STRING, SHORT_STRING, COLOR) IDENT = NUM,

/**
 * @enum linear::log::Level
 * LOG_ERR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_FULL and LOG_OFF are available.
 * @var LOG_OFF
 * no log
 * @var LOG_ERR
 * error
 * @var LOG_WARN
 * warning
 * @var LOG_INFO
 * information
 * @var LOG_DEBUG
 * debug (truncate data such as request.params, response.result etc. into 64 byte)
 * @var LOG_FULL
 * debug (not truncate data)
 **/
enum Level {
  LINEAR_LOG_LEVEL_MAP(LINEAR_LOG_LEVEL_GEN)
};

/** @cond hidden **/
#undef LINEAR_LOG_LEVEL_GEN
/** @endcond **/

/**
 * typeof callback function for linear::log
 * @param level [out] log level
 * @param file [out] file name
 * @param line [out] line number
 * @param func [out] function/method name
 * @param message [out] log message
 **/
typedef void (*LogCallback)(linear::log::Level level, const char* file, int line,
                            const char* func, const char* message);

/**
 * get log level
 * @return linear::log::Level
 **/
LINEAR_EXTERN linear::log::Level GetLevel();

/**
 * set log level
 *
 * default level is LOG_ERR
 * @param level [in] linear::log::Level level of output log
 **/
LINEAR_EXTERN void SetLevel(linear::log::Level level);

/**
 * show logs to stderr
 * @note call DisableStderr() at the end of your application
 **/
LINEAR_EXTERN bool EnableStderr();

/**
 * write log to specified file
 * @param filename [in] file name to write logs
 * @note call DisableFile() at the end of your application
 **/
LINEAR_EXTERN bool EnableFile(const std::string& filename);

/**
 * start to callback for writing logs
 * @param function [in] function name to output logs
 * @note call DisableCallback() at the end of your application
 **/
LINEAR_EXTERN bool EnableCallback(linear::log::LogCallback function);

/**
 * hide logs from stderr
 **/
LINEAR_EXTERN void DisableStderr();

/**
 * stop to write logs and close file
 **/
LINEAR_EXTERN void DisableFile();

/**
 * stop to callback for writing logs
 **/
LINEAR_EXTERN void DisableCallback();

/**
 * colorize logs
 * effects only LogStderr
 * @param flag [in] true or false
 **/
LINEAR_EXTERN void Colorize(bool flag = true);

/// @cond hidden
LINEAR_EXTERN bool DoPrint(linear::log::Level level);
LINEAR_EXTERN void Print(bool debug, linear::log::Level level, const char* file, int line, const char* func, const char* format, ...);
/// @endcond

} // namespace log

} // namespace linear

#endif // LINEAR_LOG_H_
