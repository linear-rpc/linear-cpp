#include <gtest/gtest.h>

#include <string>

#include "linear/log.h"

using namespace linear::log;

class LinearLogMacro4FunctionTest : public testing::Test {
protected:
  LinearLogMacro4FunctionTest() {}
  ~LinearLogMacro4FunctionTest() {}
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

TEST_F(LinearLogMacro4FunctionTest, showValidLog) {
  std::string TEST_DISPLAY("DISPLAY"), TEST_HIDE("HIDE");

  linear::log::EnableCallback(OnLog);
  linear::log::SetLevel(LOG_WARN);

  LINEAR_LOG(LOG_ERR, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_WARN, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_INFO, "%s\n", TEST_HIDE.c_str());
  LINEAR_LOG(LOG_DEBUG, "%s\n", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_ERR, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_DEBUG(LOG_WARN, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_DEBUG(LOG_INFO, "%s\n", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_DEBUG, "%s\n", TEST_HIDE.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
