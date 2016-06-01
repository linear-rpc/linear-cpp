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
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT + 5);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ECONNREFUSED)))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Timeout
TEST_F(WSClientServerConnectionTest, ConnectTimeout) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ETIMEDOUT)))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Cancel
TEST_F(WSClientServerConnectionTest, ConnectCancel) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect();
  WAIT_CLI_TESTED();
}

// Disconnect EALREADY
TEST_F(WSClientServerConnectionTest, DisconnectEalready) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EALREADY
TEST_F(WSClientServerConnectionTest, ConnectEalready) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  e = cs.Connect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EINVAL
TEST_F(WSClientServerConnectionTest, ConnectEinval) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  e = sh->s_.Connect();
  ASSERT_EQ(LNR_EINVAL, e.Code());
}

// Connect - Disconnect from Client in front thread
TEST_F(WSClientServerConnectionTest, DisconnectFromClientFT) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_ECONNRESET)))
    .WillOnce(Assign(&srv_connected, false));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK)))
    .WillOnce(Assign(&cli_connected, false));
 
  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  e = cs.Disconnect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_DISCONNECTED();
}

// Connect - Disconnect from Server in front thread
TEST_F(WSClientServerConnectionTest, DisconnectFromServerFT) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK)))
    .WillOnce(Assign(&srv_connected, false));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET)))
    .WillOnce(Assign(&cli_connected, false));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  sh->s_.Disconnect();
  WAIT_DISCONNECTED();
}

// Connect - Disconnect from Client in back thread
TEST_F(WSClientServerConnectionTest, DisconnectFromClientBT) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_ECONNRESET)))
    .WillOnce(DoAll(Assign(&srv_tested, true),
		    Assign(&srv_connected, false)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(Assign(&cli_connected, true),
		    WAIT_PEER_CONNECTED(&srv_connected),
		    WAIT_TEST_LOCK(&lock),
		    WithArg<0>(Disconnect())));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK)))
    .WillOnce(DoAll(Assign(&cli_tested, true),
		    Assign(&cli_connected, false)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();
  WAIT_TESTED();
  WAIT_DISCONNECTED();
}

// Connect - Disconnect from Server in back thread
TEST_F(WSClientServerConnectionTest, DisconnectFromServerBT) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch, loop);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  // Ws and Wss returns handshake response when exited OnConnect
  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh, OnMessageMock(_, _))
    .WillOnce(DoAll(Assign(&srv_tested, true),
		    WithArg<0>(Disconnect())));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK)))
    .WillOnce(Assign(&srv_connected, false));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(Assign(&cli_connected, true),
		    WAIT_TEST_LOCK(&lock),
		    WithArg<0>(SendNotify())));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET)))
    .WillOnce(DoAll(Assign(&cli_tested, true),
		    Assign(&cli_connected, false)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();
  WAIT_TESTED();
  WAIT_DISCONNECTED();
}

// Reconnect at same socket
TEST_F(WSClientServerConnectionTest, Reconnect) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).
      WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _));
    EXPECT_CALL(*sh, OnConnectMock(_))
      .WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _))
      .WillOnce(Assign(&srv_tested, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs))
      .Times(::testing::AtLeast(0));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EWS)))
      .WillOnce(WithArg<0>(Connect()));
    EXPECT_CALL(*ch, OnConnectMock(cs))
      .Times(::testing::AtLeast(0));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EWS)))
      .WillOnce(Assign(&cli_tested, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

// AutoReconnect with DigestAuthentication
ACTION(CheckDigestAuthWS) {
  Socket s = arg0;
  ASSERT_EQ(s.GetType(), Socket::WS);
  WSSocket ws = s.as<WSSocket>();
  AuthorizationContext auth = ws.GetWSRequestContext().authorization;
  ASSERT_EQ(auth.username, USER_NAME);
  ASSERT_EQ(AuthorizationContext::VALID,
            auth.Validate(PASSWORD));
  WSResponseContext ctx;
  ctx.code = LNR_WS_OK;
  ws.SetWSResponseContext(ctx);
}
TEST_F(WSClientServerConnectionTest, AutoReconnect) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh, AuthContext::DIGEST, "realm is here");
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  WSRequestContext context;
  // Digest Auth Validation (username = "user", password = "password")
  context.authenticate.username = USER_NAME;
  context.authenticate.password = PASSWORD;
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT, context);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_))
      .WillOnce(WithArg<0>(CheckDigestAuthWS()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _))
      .WillOnce(Assign(&srv_tested, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs))
      .WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK)))
      .WillOnce(Assign(&cli_tested, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

namespace global {
extern linear::Socket gs_;
}

// Connect - Disconnect delayed Socket Destruct: must not SEGV
TEST_F(WSClientServerConnectionTest, DelayedSocketDestruct) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<DelayedMockHandler> ch = linear::shared_ptr<DelayedMockHandler>(new DelayedMockHandler());
  WSClient cl(ch);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh, OnMessageMock(_, _))
    .WillOnce(DoAll(Assign(&srv_tested, true),
		    WithArg<0>(Disconnect())));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK)))
    .WillOnce(DoAll(Assign(&srv_tested, true),
		    Assign(&srv_connected, false)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(Assign(&cli_connected, true),
		    WAIT_TEST_LOCK(&lock),
		    WithArg<0>(SendNotify())));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET)))
	      .WillOnce(DoAll(Assign(&cli_tested, true),
			      Assign(&cli_connected, false)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
  WAIT_DISCONNECTED();
  ASSERT_EQ(global::gs_, cs);

  cs = cl.CreateSocket(TEST_ADDR, TEST_PORT); // unref before socket

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();
  ASSERT_NE(global::gs_, cs);
  // must not segv exit scope
}

#ifndef _WIN32
// Connect - Disconnect from other thread, and check certificate: must not SEGV
TEST_F(WSClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  shared_ptr<ThreadMockHandler> ch = linear::shared_ptr<ThreadMockHandler>(new ThreadMockHandler());
  WSClient cl(ch, loop);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_ECONNRESET)))
    .WillOnce(DoAll(Assign(&srv_tested, true),
		    Assign(&srv_connected, false)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(Assign(&cli_connected, true),
		    WAIT_PEER_CONNECTED(&srv_connected),
		    WAIT_TEST_LOCK(&lock),
		    WithArg<0>(DisconnectFromOtherThread())));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK)))
    .WillOnce(DoAll(Assign(&cli_tested, true),
		    Assign(&cli_connected, false)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());

  WAIT_CONNECTED();
  WAIT_TESTED();
  WAIT_DISCONNECTED();
  ASSERT_NE(global::gs_, cs);
}
#endif

// Connect - Stop same time
TEST_F(WSClientServerConnectionTest, ConnectStop) {
  linear::EventLoop loop;
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch, loop);
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_))
    .Times(::testing::AtLeast(0))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .Times(::testing::AtLeast(0));
#if 1
  // TODO(BUG): exists timing not called OnConnect at Server-side
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(::testing::AtLeast(0))
    .WillOnce(WithArg<0>(Disconnect()));
#else
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(::testing::AtLeast(0));
#endif
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  // TODO(BUG): exists timing not called OnConnect and OnDisconnect
  e = cs.Connect(1000);
  ASSERT_EQ(LNR_OK, e.Code());
  sv.Stop();

  WAIT_CLI_TESTED();
}
