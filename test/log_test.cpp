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

TEST_F(LinearLogTest, noLog) {
  std::string TEST_HIDE("HIDE");

  LINEAR_LOG(LOG_ERR, "%s", TEST_HIDE.c_str());
  LINEAR_LOG(LOG_WARN, "%s", TEST_HIDE.c_str());
  LINEAR_LOG(LOG_INFO, "%s", TEST_HIDE.c_str());
  LINEAR_LOG(LOG_DEBUG, "%s", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_ERR, "%s", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_WARN, "%s", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_INFO, "%s", TEST_HIDE.c_str());
  LINEAR_DEBUG(LOG_DEBUG, "%s", TEST_HIDE.c_str());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
