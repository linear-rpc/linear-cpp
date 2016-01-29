#include <gtest/gtest.h>

#include <string>

#include "log_file.h"

using namespace linear::log;

class LinearLogFileTest : public testing::Test {
protected:
  LinearLogFileTest() {}
  ~LinearLogFileTest() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(LinearLogFileTest, showValidLog) {
  std::string TEST_DISPLAY("DISPLAY");

  LogFile log;
  log.Enable("./test.log");
  log.Write(false, LOG_ERR, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_WARN, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_INFO, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_DEBUG, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
