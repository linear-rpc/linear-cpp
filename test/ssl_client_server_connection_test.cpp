#include <fstream>
#include <pthread.h>

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
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ECONNREFUSED))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Timeout
TEST_F(SSLClientServerConnectionTest, ConnectTimeout) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ETIMEDOUT))).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect(1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

// Cancel
TEST_F(SSLClientServerConnectionTest, ConnectCancel) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket("127.0.0.2", TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  Error e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect();
  WAIT_TO_FINISH_CALLBACK();
}

// Disconnect EALREADY
TEST_F(SSLClientServerConnectionTest, DisconnectEalready) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(0);

  Error e = cs.Disconnect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
  msleep(WAIT_MSEC);
}

// Connect EALREADY
TEST_F(SSLClientServerConnectionTest, ConnectEalready) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = cs.Connect();
  ASSERT_EQ(LNR_EALREADY, e.Code());
}

// Connect EINVAL
TEST_F(SSLClientServerConnectionTest, ConnectEinval) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  e = sh->s_.Connect();
  ASSERT_EQ(LNR_EINVAL, e.Code());
}

// Connect - Disconnect from Client in front thread
TEST_F(SSLClientServerConnectionTest, DisconnectFromClientFT) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectFromServerFT) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectFromClientBT) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
TEST_F(SSLClientServerConnectionTest, DisconnectFromServerBT) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);
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
TEST_F(SSLClientServerConnectionTest, Reconnect) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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
extern linear::Socket gs_;
}

// Connect - Disconnect delayed Socket Destruct: must not SEGV
TEST_F(SSLClientServerConnectionTest, DelayedSocketDestruct) {
  linear::shared_ptr<DelayedMockHandler> ch = linear::shared_ptr<DelayedMockHandler>(new DelayedMockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

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

ACTION(DisconnectFromOtherThread_SSL) {
  pthread_t thread;
  ASSERT_EQ(0, pthread_create(&thread, NULL, call_from_thread, NULL));
  pthread_join(thread, NULL);
  linear::SSLSocket ssl = global::gs_.as<linear::SSLSocket>();
  ASSERT_EQ(LNR_ENOTCONN, ssl.SetSockOpt(SOL_SOCKET, SO_KEEPALIVE, NULL, 0).Code());
  ASSERT_EQ(LNR_ENOTCONN, ssl.GetVerifyResult().Code());
  ASSERT_EQ(false, ssl.PresentPeerCertificate());
  ASSERT_THROW(ssl.GetPeerCertificate(), std::runtime_error);
}

// Connect - Disconnect from other thread, and check certificate: must not SEGV
TEST_F(SSLClientServerConnectionTest, OnConnectAndDisconnectFromOtherTherad) {
  linear::shared_ptr<ThreadMockHandler> ch = linear::shared_ptr<ThreadMockHandler>(new ThreadMockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT_PEM));
  context.SetPrivateKey(std::string(CLIENT_PKEY_PEM));
  context.SetCAFile(std::string(CA_CERT_PEM));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT_PEM));
  server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM));
  server_context.SetCAFile(std::string(CA_CERT_PEM));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_EOF))).WillOnce(Assign(&srv_finished, true));
  }

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(WithArg<0>(DisconnectFromOtherThread_SSL()));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(Assign(&cli_finished, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  ASSERT_NE(global::gs_, cs);

  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
}

ACTION(VerifySSL) {
  linear::Socket s = arg0;
  linear::SSLSocket ss = s.as<linear::SSLSocket>();
  ASSERT_EQ(linear::LNR_OK, ss.GetVerifyResult().Code());
}
// Verify Client/Server Cert
TEST_F(SSLClientServerConnectionTest, VerifyCertsPEM) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_finished, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_finished, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}
// Verify Client/Server Cert using DER format files
TEST_F(SSLClientServerConnectionTest, VerifyCertsDER) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_DER), "", SSLContext::DER));
  ASSERT_EQ(true, context.SetCAFile(std::string(CA_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_DER), "", SSLContext::DER));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_finished, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_finished, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}
// use memory for CA Cert with PEM format
TEST_F(SSLClientServerConnectionTest, VerifyCertsPEMData) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);

  std::ifstream ifs(CA_CERT_PEM);
  std::streamsize siz = ifs.seekg(0, std::ios::end).tellg();
  ifs.clear();
  ifs.seekg(0, std::ios::beg);
  unsigned char* cadata = new unsigned char[siz];
  ifs.read((char*)cadata, siz);

  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCAData(cadata, siz, SSLContext::PEM));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAData(cadata, siz, SSLContext::PEM));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_finished, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_finished, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  delete cadata;
}
// use memory for CA Cert with DER format
TEST_F(SSLClientServerConnectionTest, VerifyCertsDERData) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);

  std::ifstream ifs(CA_CERT_DER);
  std::streamsize siz = ifs.seekg(0, std::ios::end).tellg();
  ifs.clear();
  ifs.seekg(0, std::ios::beg);
  unsigned char* cadata = new unsigned char[siz];
  ifs.read((char*)cadata, siz);

  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_DER), "", SSLContext::DER));
  ASSERT_EQ(true, context.SetCAData(cadata, siz));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_DER), SSLContext::DER));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_DER), "", SSLContext::DER));
  ASSERT_EQ(true, server_context.SetCAData(cadata, siz));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_finished, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_finished, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  delete cadata;
}

#define CLIENT_CERT_W_PASS_PEM "../sample/certs/client-w-pass.pem"
#define CLIENT_PKEY_W_PASS_PEM "../sample/certs/client-w-pass.key"
#define SERVER_CERT_W_PASS_PEM "../sample/certs/server-w-pass.pem"
#define SERVER_PKEY_W_PASS_PEM "../sample/certs/server-w-pass.key"
#define PASSPHRASE             "passphrase"

// Verify Client/Server Cert
TEST_F(SSLClientServerConnectionTest, VerifyCertsWPassPEM) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_W_PASS_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_W_PASS_PEM), PASSPHRASE));
  ASSERT_EQ(true, context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_W_PASS_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_W_PASS_PEM), PASSPHRASE));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&srv_finished, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(VerifySSL()), Assign(&cli_finished, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}

ACTION(VerifyFailSSLSelfSigned) {
  linear::Socket s = arg0;
  linear::SSLSocket ss = s.as<linear::SSLSocket>();
  ASSERT_NE(linear::LNR_OK, ss.GetVerifyResult().Code());
}
// Verify Fail Client/Server Cert
TEST_F(SSLClientServerConnectionTest, VerifyFailSelfSignedAuto) {
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(0);
  EXPECT_CALL(*ch, OnConnectMock(cs)).Times(0);
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _)).WillOnce(DoAll(WithArg<0>(VerifyFailSSLSelfSigned()), Assign(&cli_finished, true)));
  srv_finished = true;
  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
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
  linear::shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, context.SetCertificate(std::string(CLIENT_CERT_PEM)));
  ASSERT_EQ(true, context.SetPrivateKey(std::string(CLIENT_PKEY_PEM)));
  ASSERT_EQ(true, context.SetCiphers(std::string(CIPHER_LIST)));
  context.SetVerifyMode(SSLContext::VERIFY_NONE);
  SSLClient cl(ch, context);
  linear::shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  SSLContext server_context(SSLContext::TLSv1_1);
  ASSERT_EQ(true, server_context.SetCertificate(std::string(SERVER_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetPrivateKey(std::string(SERVER_PKEY_PEM)));
  ASSERT_EQ(true, server_context.SetCAFile(std::string(CA_CERT_PEM)));
  ASSERT_EQ(true, server_context.SetCiphers(std::string(CIPHER_LIST)));
  server_context.SetVerifyMode(SSLContext::VERIFY_NONE);
  SSLServer sv(sh, server_context);

  Error e = sv.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());

  SSLSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(VerifyFailSSLSelfSigned_Srv()), Assign(&srv_finished, true)));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(VerifyFailSSLSelfSigned_Cli()), Assign(&cli_finished, true)));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
}
