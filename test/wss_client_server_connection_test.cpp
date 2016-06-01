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
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT + 5);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Timeout
TEST_F(WSSClientServerConnectionTest, ConnectTimeout) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Cancel
TEST_F(WSSClientServerConnectionTest, ConnectCancel) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectEalready) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EALREADY
TEST_F(WSSClientServerConnectionTest, ConnectEalready) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, ConnectEinval) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectFromClientFT) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectFromServerFT) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectFromClientBT) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, DisconnectFromServerBT) {
  linear::EventLoop loop;
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context, loop);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, Reconnect) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context, linear::AuthContext::DIGEST, "realm is here");
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
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
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
      .WillOnce(WithArg<0>(CheckDigestAuthWSS()));
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
TEST_F(WSSClientServerConnectionTest, DelayedSocketDestruct) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<DelayedMockHandler> ch = linear::shared_ptr<DelayedMockHandler>(new DelayedMockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, OnConnectAndDisconnectFromOtherThread) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<ThreadMockHandler> ch = linear::shared_ptr<ThreadMockHandler>(new ThreadMockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, linear::WSRequestContext(), context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(WSSClientServerConnectionTest, ConnectStop) {
  linear::EventLoop loop;
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, linear::WSRequestContext(), context, loop);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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

ACTION(VerifyWSS) {
  linear::Socket s = arg0;
  linear::WSSSocket wss = s.as<linear::WSSSocket>();
  ASSERT_EQ(linear::LNR_OK, wss.GetVerifyResult().Code());
}
// Verify Server Cert
TEST_F(WSSClientServerConnectionTest, VerifyServerCert) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(WithArg<0>(VerifyWSS()), Assign(&srv_tested, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(WithArg<0>(VerifyWSS()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}
// Verify Client Cert
TEST_F(WSSClientServerConnectionTest, VerifyClientCert) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, context);
  WSSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(WithArg<0>(VerifyWSS()), Assign(&srv_tested, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(WithArg<0>(VerifyWSS()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}
