#include <gtest/gtest.h>

#include <string>

#include "linear/log.h"

using namespace linear::log;

class LinearLogTest : public testing::Test {
protected:
  LinearLogTest() {}
  ~LinearLogTest() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(LinearLogTest, onoff) {
  std::string TEST_DISPLAY("DISPLAY"), TEST_HIDE("HIDE");

  linear::log::EnableStderr();
  linear::log::SetLevel(linear::log::LOG_DEBUG);
  LINEAR_LOG(LOG_ERR, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_WARN, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_INFO, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_DEBUG, "%s\n", TEST_DISPLAY.c_str());
  linear::log::SetLevel(linear::log::LOG_OFF);
  LINEAR_LOG(LOG_DEBUG, "%s\n", TEST_HIDE.c_str());
  linear::log::SetLevel(linear::log::LOG_WARN);
  LINEAR_LOG(LOG_ERR, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_WARN, "%s\n", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_INFO, "%s\n", TEST_HIDE.c_str());
  LINEAR_LOG(LOG_DEBUG, "%s\n", TEST_HIDE.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
