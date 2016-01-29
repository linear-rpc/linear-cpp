#include <gtest/gtest.h>

#include <string>

#include "log_stderr.h"

using namespace linear::log;

class LinearLogStderrTest : public testing::Test {
protected:
  LinearLogStderrTest() {}
  ~LinearLogStderrTest() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(LinearLogStderrTest, showValidLog) {
  std::string TEST_DISPLAY("DISPLAY");
  LogStderr log;
  ASSERT_FALSE(log.Enable(std::string("./test.log")));
  ASSERT_TRUE(log.Enable());
  log.Write(false, LOG_ERR, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_WARN, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_INFO, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
  log.Write(false, LOG_DEBUG, __FILE__, __LINE__, __PRETTY_FUNCTION__, TEST_DISPLAY.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
