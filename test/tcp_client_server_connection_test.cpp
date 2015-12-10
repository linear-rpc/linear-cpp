#include <pthread.h>

#include "test_common.h"

#include "linear/tcp_client.h"
#include "linear/tcp_server.h"

using namespace linear;
using ::testing::_;
using ::testing::InSequence;
using ::testing::DoAll;
using ::testing::WithArg;
using ::testing::WithArgs;
using ::testing::Eq;
using ::testing::ByRef;
using ::testing::Assign;

typedef LinearTest TCPClientServerConnectionTest;

// Refuse
TEST_F(TCPClientServerConnectionTest, ConnectRefuse) {
  MockHandler ch;
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNREFUSED))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Timeout
TEST_F(TCPClientServerConnectionTest, ConnectTimeout) {
  MockHandler ch;
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ETIMEDOUT))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Cancel
TEST_F(TCPClientServerConnectionTest, ConnectCancel) {
  MockHandler ch;
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket("127.0.0.2", TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Disconnect EALREADY
TEST_F(TCPClientServerConnectionTest, DisconnectEalready) {
  MockHandler ch;
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
  msleep(WAIT_MSEC);
}

// Connect EALREADY
TEST_F(TCPClientServerConnectionTest, ConnectEalready) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = cs.Connect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EINVAL
TEST_F(TCPClientServerConnectionTest, ConnectEinval) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = sh.s_.Connect();
  ASSERT_EQ(LNR_EINVAL, e.Code());
}

// Connect - Disconnect from Client in front thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromClientFT) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(sh, OnDisconnectMock(sh.s_, Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));

  e = cs.Disconnect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Server in front thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromServerFT) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(sh, OnDisconnectMock(sh.s_, Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));

  sh.s_.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Client in back thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromClientBT) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));;
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));;
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Server in back thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromServerBT) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);
  Socket ss;

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));;
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Reconnect at same socket
TEST_F(TCPClientServerConnectionTest, Reconnect) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), _));
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), _)).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(WithArg<0>(Connect()));
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

namespace global {
extern linear::Socket gs_;
}

// Connect - Disconnect delayed Socket Destruct: must not SEGV
TEST_F(TCPClientServerConnectionTest, DelayedSocketDestruct) {
  DelayedMockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_EQ(global::gs_, cs);

  cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_NE(global::gs_, cs);

  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
}

static void* call_from_thread(void* param) {
  global::gs_.Disconnect();
  return NULL;
}

ACTION(DisconnectFromOtherThread_TCP) {
  pthread_t thread;
  ASSERT_EQ(0, pthread_create(&thread, NULL, call_from_thread, NULL));
  pthread_join(thread, NULL);
  linear::TCPSocket tcp = global::gs_.as<linear::TCPSocket>();
  ASSERT_EQ(LNR_ENOTCONN, tcp.SetSockOpt(SOL_SOCKET, SO_KEEPALIVE, NULL, 0).Code());
}

// Connect - Disconnect from other thread, and check certificate: must not SEGV
TEST_F(TCPClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  ThreadMockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(WithArg<0>(DisconnectFromOtherThread_TCP()));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_NE(global::gs_, cs);

  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
}

// Connect - Stop same time
TEST_F(TCPClientServerConnectionTest, ConnectStop) {
  MockHandler ch;
  TCPClient cl(ch);
  MockHandler sh;
  TCPServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  // TODO(BUG): client is connected but server is not called onAccept.this bug is at tv or uv layer
  EXPECT_CALL(ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));
  srv_finished = true;

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  sv.Stop();
  WAIT_TO_FINISH_CALLBACK();
}

