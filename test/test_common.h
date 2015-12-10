#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "linear/error.h"
#include "linear/group.h"
#include "linear/handler.h"
#include "linear/log.h"
#include "linear/message.h"
#include "linear/timer.h"

// configurations
#define TEST_ADDR_4_TIMEOUT "10.123.123.123" // must not exist
#define TEST_ADDR           "127.0.0.1"      // must exist lo
#define TEST_PORT           (10001)          // must not listen
#define TEST_PORT2          (10002)          // must not listen
#define TEST_PORT3          (10003)          // must not listen
#define WAIT_MSEC           (10)             // 10 ms

#define METHOD_NAME         "echo"
#define GROUP_NAME          "test_group"

#define USER_NAME           "user"
#define PASSWORD            "password"

using namespace linear::log;

/* milliseconds sleep function */
unsigned int msleep(unsigned int milliseconds);

// Test Base Class
class LinearTest : public ::testing::Test {
 public:
 LinearTest() : srv_finished(false), cli_finished(false) {}
  virtual ~LinearTest() {}

  virtual void SetUp() {
    linear::log::SetLevel(linear::log::LOG_DEBUG);
    linear::log::EnableStderr();
  }
  virtual void TearDown() {}
  virtual void WAIT_TO_FINISH_CALLBACK() {
    while (!srv_finished || !cli_finished) {
      msleep(1);
    }
    cli_finished = false;
    srv_finished = false;
  }

  bool srv_finished;
  bool cli_finished;
};

// Test Mock
class MockHandler : public linear::Handler {
 public:
  MOCK_METHOD1(OnConnectMock,    void(const linear::Socket& s));
  MOCK_METHOD2(OnDisconnectMock, void(const linear::Socket& s, const linear::Error& e));
  MOCK_METHOD2(OnMessageMock,    void(const linear::Socket& s, const linear::Message& m));
  MOCK_METHOD3(OnErrorMock,      void(const linear::Socket& s, const linear::Message& m, const linear::Error& e));

  MockHandler() : m_(NULL), err_m_(NULL) {}
  virtual ~MockHandler() {
    if (m_) {
      delete m_;
    }
    if (err_m_) {
      delete err_m_;
    }
  }
  void OnConnect(const linear::Socket& s) {
    s_ = s;
    OnConnectMock(s);
  }
  void OnDisconnect(const linear::Socket& s, const linear::Error& e) {
    OnDisconnectMock(s, e);
    s_ = linear::Socket(); // unref
  }
  void OnMessage(const linear::Socket& s, const linear::Message& m) {
    if (m_) {
      delete m_;
      m_ = NULL;
    }
    switch(m.type) {
    case linear::REQUEST:
      m_ = new linear::Request(m.as<linear::Request>());
      break;
    case linear::RESPONSE:
      m_ = new linear::Response(m.as<linear::Response>());
      break;
    case linear::NOTIFY:
      m_ = new linear::Notify(m.as<linear::Notify>());
      break;
    default:
      ASSERT_FALSE("Invalid Message");
      break;
    }
    OnMessageMock(s, m);
  }
  void OnError(const linear::Socket& s, const linear::Message& m, const linear::Error& e) {
    if (err_m_) {
      delete err_m_;
      err_m_ = NULL;
    }
    switch(m.type) {
    case linear::REQUEST:
      err_m_ = new linear::Request(m.as<linear::Request>());
      break;
    case linear::RESPONSE:
      err_m_ = new linear::Response(m.as<linear::Response>());
      break;
    case linear::NOTIFY:
      err_m_ = new linear::Notify(m.as<linear::Notify>());
      break;
    default:
      ASSERT_FALSE("Invalid Message");
      break;
    }
    OnErrorMock(s, m, e);
  }

  linear::Socket s_;
  linear::Message* m_;
  linear::Message* err_m_;
};

class DelayedMockHandler : public linear::Handler {
 public:
  MOCK_METHOD1(OnConnectMock,    void(const linear::Socket& s));
  MOCK_METHOD2(OnDisconnectMock, void(const linear::Socket& s, const linear::Error& e));
  DelayedMockHandler() {}
  virtual ~DelayedMockHandler() {}
  void OnConnect(const linear::Socket& s);
  void OnDisconnect(const linear::Socket& s, const linear::Error& e);
};

class ThreadMockHandler : public linear::Handler {
 public:
  MOCK_METHOD1(OnConnectMock,    void(const linear::Socket& s));
  MOCK_METHOD2(OnDisconnectMock, void(const linear::Socket& s, const linear::Error& e));
  ThreadMockHandler() {}
  virtual ~ThreadMockHandler() {}
  void OnConnect(const linear::Socket& s);
  void OnDisconnect(const linear::Socket& s, const linear::Error& e);
};

// Param
struct Params {
  Params()
    : a(1), b(0), c(-1), d(0.5), e(std::string("test")), f(true),
      g(std::vector<int>()), h(std::map<std::string, std::string>()) {}
  ~Params() {}

  int a, b, c;
  float d;
  std::string e;
  bool f;
  std::vector<int> g;
  std::map<std::string, std::string> h;

  LINEAR_PACK(a, b, c, d, e, f, g, h);
};

// ACTION
ACTION(SendRequest) {
  linear::Socket s = arg0;
  linear::Request request(std::string(METHOD_NAME), Params());
  linear::Error e = request.Send(s);
  ASSERT_EQ(linear::LNR_OK, e.Code());
}
ACTION(SendResponse) {
  linear::Socket s = arg0;
  const linear::Message& m = arg1;
  switch (m.type) {
  case linear::REQUEST: {
    const linear::Request& req = m.as<linear::Request>();
    linear::Response resp(req.msgid, req.params);
    resp.Send(s);
    break;
  }
  default:
    ASSERT_FALSE("Invalid Message Received");
    break;
  }
}
ACTION(SendNotify) {
  linear::Socket s = arg0;
  linear::Notify notify(std::string(METHOD_NAME), Params());
  linear::Error e = notify.Send(s);
  ASSERT_EQ(linear::LNR_OK, e.Code());
}
ACTION(SendNotify2Group) {
  linear::Notify notify(std::string(METHOD_NAME), Params());
  notify.Send(GROUP_NAME);
}
ACTION(JoinToGroup) {
  linear::Socket s = arg0;
  linear::Group::Join(GROUP_NAME, s);
}
ACTION(LeaveFromGroup) {
  linear::Socket s = arg0;
  linear::Group::Leave(GROUP_NAME, s);
}
ACTION(Connect) {
  linear::Socket s = arg0;
  s.Connect();
}
ACTION(Disconnect) {
  linear::Socket s = arg0;
  s.Disconnect();
}

#endif // TEST_COMMON_H_
