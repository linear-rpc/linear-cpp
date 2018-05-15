#include "test_common.h"

#include "linear/ssl_client.h"
#include "linear/ssl_server.h"

#define CLIENT_CERT_PEM "../sample/certs/client.pem"
#define CLIENT_PKEY_PEM "../sample/certs/client.key"
#define CLIENT_CERT_DER "../sample/certs/client.der"
#define CLIENT_PKEY_DER "../sample/certs/client.key.der"
#define SERVER_CERT_PEM "../sample/certs/server.pem"
#define SERVER_PKEY_PEM "../sample/certs/server.key"
#define SERVER_CERT_DER "../sample/certs/server.der"
#define SERVER_PKEY_DER "../sample/certs/server.key.der"
#define CIPHER_LIST     "ALL:EECDH+HIGH:EDH+HIGH:+MEDIUM+HIGH:!EXP:!LOW:!eNULL:!aNULL:!MD5:!RC4:!ADH:!KRB5:!PSK:!SRP"
#define CA_CERT_PEM     "../sample/certs/ca.pem"
#define CA_CERT_DER     "../sample/certs/ca.der"
#define CA_CERT_PATH    "./CACert"

using namespace linear;
using ::testing::_;
using ::testing::InSequence;
using ::testing::DoAll;
using ::testing::WithArg;
using ::testing::WithArgs;
using ::testing::Eq;
using ::testing::ByRef;
using ::testing::Assign;

typedef LinearTest SSLClientServerConnectionTest;

// Refuse
TEST_F(SSLClientServerConnectionTest, ConnectRefuse) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT + 5);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Timeout
TEST_F(SSLClientServerConnectionTest, ConnectTimeout) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  // sometimes returned LNR_EADDRNOTAVAIL...
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

// Cancel
TEST_F(SSLClientServerConnectionTest, ConnectCancel) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectEalready) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EALREADY
TEST_F(SSLClientServerConnectionTest, ConnectEalready) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, ConnectEinval) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectFromClientFT) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectFromServerFT) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectFromClientBT) {
  linear::EventLoop loop;
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context, loop);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectFromServerBT) {
  linear::EventLoop loop;
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context, loop);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, Reconnect) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DelayedSocketDestruct) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<DelayedMockHandler> ch = linear::shared_ptr<DelayedMockHandler>(new DelayedMockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
// Connect - Disconnect from other thread, and check certificate: must not SEGV
TEST_F(SSLClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  linear::EventLoop loop;
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<ThreadMockHandler> ch = linear::shared_ptr<ThreadMockHandler>(new ThreadMockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context, loop);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, ConnectStop) {
  linear::EventLoop loop;
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context, loop);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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

ACTION(VerifySSL) {
  linear::Socket s = arg0;
  linear::SSLSocket ss = s.as<linear::SSLSocket>();
  ASSERT_EQ(linear::LNR_OK, ss.GetVerifyResult().Code());
}
// Verify Client/Server Cert
TEST_F(SSLClientServerConnectionTest, VerifyCertsPEM) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_tested, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}
// Verify Client/Server Cert using DER format files
TEST_F(SSLClientServerConnectionTest, VerifyCertsDER) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_DER), "", SSLContext::DER));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_DER), "", SSLContext::DER));
  ASSERT_EQ(true, context.SetCAFile(std::string(CA_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_tested, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}
// Verify Client/Server Cert using CAPath
TEST_F(SSLClientServerConnectionTest, VerifyCertsCAPath) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAPath(CA_CERT_PATH));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCAPath(CA_CERT_PATH));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_tested, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

#define CLIENT_CERT_W_PASS_PEM "../sample/certs/client-w-pass.pem"
#define CLIENT_PKEY_W_PASS_PEM "../sample/certs/client-w-pass.key"
#define SERVER_CERT_W_PASS_PEM "../sample/certs/server-w-pass.pem"
#define SERVER_PKEY_W_PASS_PEM "../sample/certs/server-w-pass.key"
#define PASSPHRASE             "passphrase"

// Verify Client/Server Cert
TEST_F(SSLClientServerConnectionTest, VerifyCertsWPassPEM) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_W_PASS_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_W_PASS_PEM), PASSPHRASE));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_W_PASS_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_W_PASS_PEM), PASSPHRASE));
  ASSERT_EQ(true, context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_tested, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}

ACTION(VerifyFailSSLSelfSigned) {
  linear::Socket s = arg0;
  linear::SSLSocket ss = s.as<linear::SSLSocket>();
  ASSERT_NE(linear::LNR_OK, ss.GetVerifyResult().Code());
}
// Verify Fail Client/Server Cert
TEST_F(SSLClientServerConnectionTest, VerifyFailSelfSignedAuto) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .Times(0);
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(DoAll(WithArg<0>(VerifyFailSSLSelfSigned()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();
}

ACTION(VerifyFailSSLSelfSigned_Srv) {
  linear::Socket s = arg0;
  linear::SSLSocket ss = s.as<linear::SSLSocket>();
  try {
    std::vector<linear::X509Certificate> certs = ss.GetPeerCertificateChain();
    ASSERT_FALSE("certs exists");
  } catch(...) {}
  ASSERT_NE(linear::LNR_OK, ss.GetVerifyResult().Code());
}
ACTION(VerifyFailSSLSelfSigned_Cli) {
  linear::Socket s = arg0;
  linear::SSLSocket ss = s.as<linear::SSLSocket>();
  std::vector<linear::X509Certificate> certs = ss.GetPeerCertificateChain();
  ASSERT_EQ(2, certs.size());
  ASSERT_NE(linear::LNR_OK, ss.GetVerifyResult().Code());
}
// Verify Fail Client/Server Cert
TEST_F(SSLClientServerConnectionTest, VerifyFailSelfSigned) {
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext server_context(SSLContext::TLSv1_1);
#else
  SSLContext server_context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_NONE);
  SSLServer sv(sh, server_context);
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSLContext context(SSLContext::TLSv1_1);
#else
  SSLContext context(SSLContext::TLS);
#endif
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_NONE);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(WithArg<0>(VerifyFailSSLSelfSigned_Srv()), Assign(&srv_tested, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(DoAll(WithArg<0>(VerifyFailSSLSelfSigned_Cli()), Assign(&cli_tested, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
}
