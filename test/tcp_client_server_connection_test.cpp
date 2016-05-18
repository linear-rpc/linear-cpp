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
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT + 5);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ECONNREFUSED))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Timeout
TEST_F(TCPClientServerConnectionTest, ConnectTimeout) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ETIMEDOUT))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Cancel
TEST_F(TCPClientServerConnectionTest, ConnectCancel) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket("127.0.0.2", TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Disconnect EALREADY
TEST_F(TCPClientServerConnectionTest, DisconnectEalready) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
  msleep(WAIT_MSEC);
}

// Connect EALREADY
TEST_F(TCPClientServerConnectionTest, ConnectEalready) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = cs.Connect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EINVAL
TEST_F(TCPClientServerConnectionTest, ConnectEinval) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = sh->s_.Connect();
  ASSERT_EQ(LNR_EINVAL, e.Code());
}

// Connect - Disconnect from Client in front thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromClientFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*sh, OnDisconnectMock(sh->s_, Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));

  e = cs.Disconnect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Server in front thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromServerFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*sh, OnDisconnectMock(sh->s_, Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));

  sh->s_.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Client in back thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromClientBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));;
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));;
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Server in back thread
TEST_F(TCPClientServerConnectionTest, DisconnectFromServerBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);
  Socket ss;

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));;
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Reconnect at same socket
TEST_F(TCPClientServerConnectionTest, Reconnect) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _));
    EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _)).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(WithArg<0>(Connect()));
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

namespace global {
extern Socket gs_;
}

// Connect - Disconnect delayed Socket Destruct: must not SEGV
TEST_F(TCPClientServerConnectionTest, DelayedSocketDestruct) {
  shared_ptr<DelayedMockHandler> ch = linear::shared_ptr<DelayedMockHandler>(new DelayedMockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_EQ(global::gs_, cs);

  cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_NE(global::gs_, cs);

  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
}

static void* call_from_thread(void* param) {
  global::gs_.Disconnect();
  return NULL;
}

ACTION(DisconnectFromOtherThread_TCP) {
  pthread_t thread;
  ASSERT_EQ(0, pthread_create(&thread, NULL, call_from_thread, NULL));
  pthread_join(thread, NULL);
  TCPSocket tcp = global::gs_.as<TCPSocket>();
  ASSERT_EQ(LNR_ENOTCONN, tcp.SetSockOpt(SOL_SOCKET, SO_KEEPALIVE, NULL, 0).Code());
}

// Connect - Disconnect from other thread, and check certificate: must not SEGV
TEST_F(TCPClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  shared_ptr<ThreadMockHandler> ch = linear::shared_ptr<ThreadMockHandler>(new ThreadMockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));
  }

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());
  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(WithArg<0>(DisconnectFromOtherThread_TCP()));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_NE(global::gs_, cs);

  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
}

// Connect - Stop same time
TEST_F(TCPClientServerConnectionTest, ConnectStop) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  // TODO(BUG): client is connected but server is not called onAccept.this bug is at tv or uv layer
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));
  srv_finished = true;

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  sv.Stop();
  WAIT_TO_FINISH_CALLBACK();
}

// use ClientLoop
TEST_F(TCPClientServerConnectionTest, ClientLoop) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch, loop);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// use ServerLoop
TEST_F(TCPClientServerConnectionTest, ServerLoop) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh, loop);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  sv.Stop();
}

// ClientLoop on Global
linear::EventLoop g_loop;
TEST_F(TCPClientServerConnectionTest, ClientLoopOnGlobal) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch, g_loop);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  sv.Stop();
}

// ServerLoop on Global
TEST_F(TCPClientServerConnectionTest, ServerLoopOnGlobal) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh, g_loop);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT + 1);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT + 1);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  sv.Stop();
}

// Client on Global
TCPClient *g_cl;
TEST_F(TCPClientServerConnectionTest, ClientOnGlobal) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  g_cl = new TCPClient(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  linear::TCPSocket cs = g_cl->CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  delete g_cl;
}

// Server on Global
TCPServer* g_sv;
TEST_F(TCPClientServerConnectionTest, ServerOnGlobal) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  g_sv = new TCPServer(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = g_sv->Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  linear::TCPSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  delete g_sv;
}

// Socket on Global
linear::TCPSocket g_cs;
TEST_F(TCPClientServerConnectionTest, SocketOnGlobal) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  g_cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(g_cs)).Times(::testing::AtLeast(0)).WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(g_cs, _)).WillOnce(Assign(&cli_finished, true));

  e = g_cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Connect - Disconnect from Client in front thread ipv6
TEST_F(TCPClientServerConnectionTest, DisconnectFromClientFTv6) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

  Error e = sv.Start("::1", TEST_PORT);
  if (e == Error(LNR_EADDRNOTAVAIL)) {
    LINEAR_LOG(LOG_DEBUG, "not support v6 network");
    return;
  }
  ASSERT_EQ(LNR_OK, e.Code());

  TCPSocket cs = cl.CreateSocket("::1", TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*sh, OnDisconnectMock(sh->s_, Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));

  e = cs.Disconnect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

