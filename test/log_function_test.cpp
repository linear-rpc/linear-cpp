#include <gtest/gtest.h>

#include <string>

#include "log_function.h"

using namespace linear::log;

class LinearLogFunctionTest : public testing::Test {
protected:
  LinearLogFunctionTest() {}
  ~LinearLogFunctionTest() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

void OnLog(linear::log::Level level, const char* file, int line, const char* func, const char* data) {
  static const char *l;

  switch (level) {
  case LOG_ERR:
    l = "ERROR";
    break;
  case LOG_WARN:
    l = "WARNING";
    break;
  case LOG_INFO:
    l = "INFO";
    break;
  case LOG_DEBUG:
    l = "DEBUG";
    break;
  default:
    l = "DEFAULT";
    break;
  }
  std::cerr << "[" << l << "] (" << file << ":" << line << ", " << func << "): " << data;
}

TEST_F(LinearLogFunctionTest, showValidLog) {
  std::string TEST_DISPLAY("DISPLAY\n");

  LogFunction log;
  log.Enable(OnLog);
  log.Write(false, LOG_ERR, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_WARN, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_INFO, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_DEBUG, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
