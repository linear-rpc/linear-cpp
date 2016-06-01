#ifndef _WIN32
# include <pthread.h>
#endif

#include "test_common.h"

namespace global {
  linear::Socket gs_;
}

void* called_by_some_thread(void* param) {
  global::gs_.Disconnect();
  return NULL;
}

void DelayedMockHandler::OnConnect(const linear::Socket& s) {
  global::gs_ = linear::Socket();
  OnConnectMock(s);
}
void DelayedMockHandler::OnDisconnect(const linear::Socket& s, const linear::Error& e) {
  global::gs_ = s;
  OnDisconnectMock(s, e);
}

void ThreadMockHandler::OnConnect(const linear::Socket& s) {
  global::gs_ = s;
  OnConnectMock(s);
}
void ThreadMockHandler::OnDisconnect(const linear::Socket& s, const linear::Error& e) {
  global::gs_ = linear::Socket();
  OnDisconnectMock(s, e);
}

#if defined(_WIN32)
# include <windows.h>
unsigned int msleep(unsigned int milliseconds) {
  Sleep(milliseconds);
  return 0;
}
#else
# include <unistd.h>
unsigned int msleep(unsigned int milliseconds) {
  usleep(milliseconds * 1000);
  return 0;
}
#endif
