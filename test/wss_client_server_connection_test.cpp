#include <pthread.h>

#include "test_common.h"

#include "linear/wss_client.h"
#include "linear/wss_server.h"

#define CLIENT_CERT          "../sample/certs/client.pem"
#define CLIENT_PKEY          "../sample/certs/client.key"
#define SERVER_CERT          "../sample/certs/server.pem"
#define SERVER_PKEY          "../sample/certs/server.key"
#define CIPHER_LIST          "AES128-GCM-SHA256:RC4:HIGH:!MD5:!aNULL:!EDH"
#define CA_CERT              "../sample/certs/ca.pem"

using namespace linear;
using ::testing::_;
using ::testing::InSequence;
using ::testing::DoAll;
using ::testing::WithArg;
using ::testing::WithArgs;
using ::testing::Eq;
using ::testing::ByRef;
using ::testing::Assign;

typedef LinearTest WSSClientServerConnectionTest;

// Refuse
TEST_F(WSSClientServerConnectionTest, ConnectRefuse) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNREFUSED))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Timeout
TEST_F(WSSClientServerConnectionTest, ConnectTimeout) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ETIMEDOUT))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Cancel
TEST_F(WSSClientServerConnectionTest, ConnectCancel) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket("127.0.0.2", TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Disconnect EALREADY
TEST_F(WSSClientServerConnectionTest, DisconnectEalready) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
  msleep(WAIT_MSEC);
}

// Connect EALREADY
TEST_F(WSSClientServerConnectionTest, ConnectEalready) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = cs.Connect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EINVAL
TEST_F(WSSClientServerConnectionTest, ConnectEinval) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = sh.s_.Connect();
  ASSERT_EQ(LNR_EINVAL, e.Code());
}

// Connect - Disconnect from Client in front thread
TEST_F(WSSClientServerConnectionTest, DisconnectFromClientFT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectFromServerFT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectFromClientBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectFromServerBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);
  Socket ss;

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

// Reconnect at same socket
TEST_F(WSSClientServerConnectionTest, Reconnect) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), _));
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), _)).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    // never called OnConnect: fail handshake
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET))).WillOnce(WithArg<0>(Connect()));
    // never called OnConnect: fail handshake
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// AutoReconnect with DigestAuthentication
ACTION(CheckDigestAuthWSS) {
  linear::Socket s = arg0;
  ASSERT_EQ(s.GetType(), linear::Socket::WSS);
  linear::WSSSocket wss = s.as<WSSSocket>();
  linear::AuthorizationContext auth = wss.GetWSRequestContext().authorization;
  ASSERT_EQ(auth.username, USER_NAME);
  ASSERT_EQ(linear::AuthorizationContext::VALID,
            auth.Validate(PASSWORD));
  linear::WSResponseContext ctx;
  ctx.code = LNR_WS_OK;
  wss.SetWSResponseContext(ctx);
}
TEST_F(WSSClientServerConnectionTest, AutoReconnect) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  linear::WSRequestContext ws_context;
  // Digest Auth Validation (username = "user", password = "password")
  ws_context.authenticate.username = USER_NAME;
  ws_context.authenticate.password = PASSWORD;
  WSSClient cl(ch, ws_context, context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context, linear::AuthContext::DIGEST, "realm is here");

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(CheckDigestAuthWSS()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), _)).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

namespace global {
extern linear::Socket gs_;
}

// Connect - Disconnect delayed Socket Destruct: must not SEGV
TEST_F(WSSClientServerConnectionTest, DelayedSocketDestruct) {
  DelayedMockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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

ACTION(DisconnectFromOtherThread_WSS) {
  pthread_t thread;
  ASSERT_EQ(0, pthread_create(&thread, NULL, call_from_thread, NULL));
  pthread_join(thread, NULL);
  linear::WSSSocket wss = global::gs_.as<linear::WSSSocket>();
  ASSERT_EQ(LNR_ENOTCONN, wss.SetSockOpt(SOL_SOCKET, SO_KEEPALIVE, NULL, 0).Code());
  ASSERT_EQ(LNR_ENOTCONN, wss.GetVerifyResult().Code());
  ASSERT_EQ(false, wss.PresentPeerCertificate());
  ASSERT_THROW(wss.GetPeerCertificate(), std::runtime_error);
}

// Connect - Disconnect from other thread, and check certificate: must not SEGV
TEST_F(WSSClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  ThreadMockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, linear::WSRequestContext(), context);
  MockHandler sh;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_ECONNRESET))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(WithArg<0>(DisconnectFromOtherThread_WSS()));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_NE(global::gs_, cs);

  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
}
