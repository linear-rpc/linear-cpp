#include <gtest/gtest.h>

#ifdef _LINEAR_LOG_FCGI_
# include <stdio.h>
FILE* gStdout = stdout;
FILE* gStderr = stderr;
#include <fcgi_stdio.h>
#endif

#include <string>

#include "linear/log.h"

using namespace linear::log;

class LinearLogMacro4StderrTest : public testing::Test {
protected:
  LinearLogMacro4StderrTest() {}
  ~LinearLogMacro4StderrTest() {}
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(LinearLogMacro4StderrTest, showValidLog) {
  std::string TEST_DISPLAY("DISPLAY"), TEST_HIDE("HIDE");

#ifdef _LINEAR_LOG_FCGI_
  _fcgi_sF[1].stdio_stream = gStdout;
  _fcgi_sF[2].stdio_stream = gStderr;
#endif

  ASSERT_EQ(true, linear::log::EnableStderr());
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
