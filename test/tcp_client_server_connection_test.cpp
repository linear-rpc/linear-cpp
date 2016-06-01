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
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT + 5);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Timeout
TEST_F(TCPClientServerConnectionTest, ConnectTimeout) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Cancel
TEST_F(TCPClientServerConnectionTest, ConnectCancel) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

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
TEST_F(TCPClientServerConnectionTest, DisconnectEalready) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EALREADY
TEST_F(TCPClientServerConnectionTest, ConnectEalready) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(TCPClientServerConnectionTest, ConnectEinval) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(TCPClientServerConnectionTest, DisconnectFromClientFT) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_EOF)))
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
TEST_F(TCPClientServerConnectionTest, DisconnectFromServerFT) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF)))
    .WillOnce(Assign(&cli_connected, false));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  sh->s_.Disconnect();
  WAIT_DISCONNECTED();
}

// Connect - Disconnect from Client in back thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromClientBT) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch, loop);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_EOF)))
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
TEST_F(TCPClientServerConnectionTest, DisconnectFromServerBT) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch, loop);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(Assign(&srv_connected, true),
		    WAIT_PEER_CONNECTED(&cli_connected),
		    WAIT_TEST_LOCK(&lock),
		    WithArg<0>(Disconnect())));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK)))
    .WillOnce(DoAll(Assign(&srv_tested, true),
		    Assign(&srv_connected, false)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF)))
    .WillOnce(DoAll(Assign(&cli_tested, true),
		    Assign(&cli_connected, false)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();
  WAIT_TESTED();
  WAIT_DISCONNECTED();
}

// Reconnect at same socket
TEST_F(TCPClientServerConnectionTest, Reconnect) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF)))
      .WillOnce(WithArg<0>(Connect()));
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF)))
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
TEST_F(TCPClientServerConnectionTest, DelayedSocketDestruct) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<DelayedMockHandler> ch = linear::shared_ptr<DelayedMockHandler>(new DelayedMockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK)))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF)))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
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
// Connect - Disconnect from other thread: must not SEGV
TEST_F(TCPClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<ThreadMockHandler> ch = linear::shared_ptr<ThreadMockHandler>(new ThreadMockHandler());
  TCPClient cl(ch, loop);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_EOF)))
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
TEST_F(TCPClientServerConnectionTest, ConnectStop) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
  // TODO(BUG): client is connected but server is not called onAccept.this bug is at tv or uv layer
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(::testing::AtLeast(0))
    .WillOnce(WithArg<0>(Disconnect()));
#else
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(::testing::AtLeast(0));
#endif
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  sv.Stop();

  WAIT_CLI_TESTED();
}

// use ClientLoop
TEST_F(TCPClientServerConnectionTest, ClientLoop) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch, loop);
  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

// use ServerLoop
TEST_F(TCPClientServerConnectionTest, ServerLoop) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh, loop);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

// ClientLoop on Global
linear::EventLoop g_loop;

TEST_F(TCPClientServerConnectionTest, ClientLoopOnGlobal) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch, g_loop);
  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

// ServerLoop on Global
TEST_F(TCPClientServerConnectionTest, ServerLoopOnGlobal) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh, g_loop);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

// Socket on Global
linear::TCPSocket g_cs;

TEST_F(TCPClientServerConnectionTest, SocketOnGlobal) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  g_cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(g_cs))
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(g_cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = g_cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

// Connect - IPv6
TEST_F(TCPClientServerConnectionTest, IPv6) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start("::1", TEST_PORT);
    if (e == Error(LNR_EADDRNOTAVAIL)) {
      LINEAR_LOG(LOG_DEBUG, "IPv6 not supported");
      return;
    }
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket("::1", TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}
