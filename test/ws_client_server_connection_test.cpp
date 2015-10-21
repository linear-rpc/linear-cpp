#include <pthread.h>

#include "test_common.h"

#include "linear/ws_client.h"
#include "linear/ws_server.h"

using namespace linear;
using ::testing::_;
using ::testing::InSequence;
using ::testing::DoAll;
using ::testing::WithArg;
using ::testing::WithArgs;
using ::testing::Eq;
using ::testing::ByRef;
using ::testing::Assign;

typedef LinearTest WSClientServerConnectionTest;

// Refuse
TEST_F(WSClientServerConnectionTest, ConnectRefuse) {
  MockHandler ch;
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNREFUSED))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Timeout
TEST_F(WSClientServerConnectionTest, ConnectTimeout) {
  MockHandler ch;
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ETIMEDOUT))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Cancel
TEST_F(WSClientServerConnectionTest, ConnectCancel) {
  MockHandler ch;
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket("127.0.0.2", TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Disconnect EALREADY
TEST_F(WSClientServerConnectionTest, DisconnectEalready) {
  MockHandler ch;
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
  msleep(WAIT_MSEC);
}

// Connect EALREADY
TEST_F(WSClientServerConnectionTest, ConnectEalready) {
  MockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = cs.Connect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EINVAL
TEST_F(WSClientServerConnectionTest, ConnectEinval) {
  MockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = sh.s_.Connect();
  ASSERT_EQ(LNR_EINVAL, e.Code());
}

// Connect - Disconnect from Client in front thread
TEST_F(WSClientServerConnectionTest, DisconnectFromClientFT) {
  MockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(sh, OnDisconnectMock(sh.s_, Error(LNR_ECONNRESET))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));

  e = cs.Disconnect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Server in front thread
TEST_F(WSClientServerConnectionTest, DisconnectFromServerFT) {
  MockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(sh, OnDisconnectMock(sh.s_, Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET))).WillOnce(Assign(&cli_finished, true));

  sh.s_.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Client in back thread
TEST_F(WSClientServerConnectionTest, DisconnectFromClientBT) {
  MockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);
  Socket ss;

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_ECONNRESET))).WillOnce(Assign(&srv_finished, true));;
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
TEST_F(WSClientServerConnectionTest, DisconnectFromServerBT) {
  MockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    // never called OnConnect: fail handshake
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET))).WillOnce(Assign(&cli_finished, true));;
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

namespace global {
extern linear::Socket gs_;
}

// Connect - Disconnect delayed Socket Destruct: must not SEGV
TEST_F(WSClientServerConnectionTest, DelayedSocketDestruct) {
  DelayedMockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    // never called OnConnect: fail handshake
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET))).WillOnce(Assign(&cli_finished, true));
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

ACTION(DisconnectFromOtherThread_WS) {
  pthread_t thread;
  ASSERT_EQ(0, pthread_create(&thread, NULL, call_from_thread, NULL));
  pthread_join(thread, NULL);
  linear::WSSocket ws = global::gs_.as<linear::WSSocket>();
  ASSERT_EQ(LNR_ENOTCONN, ws.SetSockOpt(SOL_SOCKET, SO_KEEPALIVE, NULL, NULL).Code());
}

// Connect - Disconnect from other thread, and check certificate: must not SEGV
TEST_F(WSClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  ThreadMockHandler ch;
  WSClient cl(ch);
  MockHandler sh;
  WSServer sv(sh);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_ECONNRESET))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(WithArg<0>(DisconnectFromOtherThread_WS()));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_NE(global::gs_, cs);

  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
}
