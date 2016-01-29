#include <gtest/gtest.h>

#include <string>

#include "linear/log.h"

using namespace linear::log;

class LinearLogMacro4FileTest : public testing::Test {
protected:
  LinearLogMacro4FileTest() {}
  ~LinearLogMacro4FileTest() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(LinearLogMacro4FileTest, showValidLog) {
  std::string TEST_DISPLAY("DISPLAY"), TEST_HIDE("HIDE");

  ASSERT_EQ(true, linear::log::EnableFile("./test.log"));
  linear::log::SetLevel(LOG_WARN);

  LINEAR_LOG(LOG_ERR, "%s", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_WARN, "%s", TEST_DISPLAY.c_str());
  LINEAR_LOG(LOG_INFO, "%s", TEST_HIDE.c_str());
  LINEAR_LOG(LOG_DEBUG, "%s", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_ERR, "%s", TEST_DISPLAY.c_str());
  LINEAR_DEBUG(LOG_WARN, "%s", TEST_DISPLAY.c_str());
  LINEAR_DEBUG(LOG_INFO, "%s", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_DEBUG, "%s", TEST_HIDE.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
