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
  LinearTest()
    : lock(false),
      srv_connected(false), cli_connected(false),
      srv_tested(false), cli_tested(false) {}
  virtual ~LinearTest() {}

  virtual void SetUp() {
    linear::log::SetLevel(linear::log::LOG_DEBUG);
    linear::log::EnableStderr();
  }
  virtual void TearDown() {
    linear::log::DisableStderr();
  }
  virtual void WAIT_CONNECTED() {
    while (!srv_connected || !cli_connected) {
      msleep(1);
    }
  }
  virtual void WAIT_DISCONNECTED() {
    while (srv_connected || cli_connected) {
      msleep(1);
    }
  }
  virtual void WAIT_TESTED() {
    while (!srv_tested || !cli_tested) {
      lock = true;
      msleep(1);
    }
    lock = false;
  }
  virtual void WAIT_SRV_TESTED() {
    while (!srv_tested) {
      msleep(1);
    }
  }
  virtual void WAIT_CLI_TESTED() {
    while (!cli_tested) {
      msleep(1);
    }
  }
  bool lock;
  bool srv_connected;
  bool cli_connected;
  bool srv_tested;
  bool cli_tested;
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

 public:
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

class BlockMockHandler : public linear::Handler {
 public:
  MOCK_METHOD1(OnConnectMock,    void(const linear::Socket& s));
  MOCK_METHOD2(OnDisconnectMock, void(const linear::Socket& s, const linear::Error& e));
  MOCK_METHOD2(OnMessageMock,    void(const linear::Socket& s, const linear::Message& m));
  MOCK_METHOD3(OnErrorMock,      void(const linear::Socket& s, const linear::Message& m, const linear::Error& e));
  BlockMockHandler() : do_block(true) {}
  virtual ~BlockMockHandler() {}
  void OnConnect(const linear::Socket& s) {
    OnConnectMock(s);
  }
  void OnDisconnect(const linear::Socket& s, const linear::Error& e) {
    OnDisconnectMock(s, e);
  }
  void OnMessage(const linear::Socket& s, const linear::Message& m) {
    while (do_block) {
      msleep(1);
    }
    OnMessageMock(s, m);
  }
  void OnError(const linear::Socket& s, const linear::Message& m, const linear::Error& e) {
    OnErrorMock(s, m, e);
  }
  bool do_block;
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
ACTION_P(WAIT_TEST_LOCK, l) {
  while (!*l) {
    msleep(1);
  }
}
ACTION_P(WAIT_PEER_CONNECTED, f) {
  while(!*f) {
    msleep(1);
  }
}
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
ACTION_P2(MultiSendNotify, num, limit) {
  linear::Socket s = arg0;
  if (limit > 0) {
    s.SetMaxSendBufferSize(limit);
  }
  linear::Notify notify("test", std::string(10000, 'a'));
  for (int i = 0; i < num; i++) {
    linear::Error e = notify.Send(s);
    ASSERT_EQ(linear::Error(linear::LNR_OK), e);
  }
}
ACTION(CheckEbusy) {
  linear::Error e = arg0;
  ASSERT_EQ(linear::Error(linear::LNR_EBUSY), e);
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

namespace global {
extern linear::Socket gs_;
}

#ifndef _WIN32
void* called_by_some_thread(void* param);

ACTION(DisconnectFromOtherThread) {
  pthread_t thread;
  ASSERT_EQ(0, pthread_create(&thread, NULL, called_by_some_thread, NULL));
  pthread_join(thread, NULL);
  ASSERT_EQ(true,
	    linear::Socket::DISCONNECTING == global::gs_.GetState() ||
	    linear::Socket::DISCONNECTED == global::gs_.GetState());
}
#endif

#endif // TEST_COMMON_H_
