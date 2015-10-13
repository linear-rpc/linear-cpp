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

typedef LinearTest WSSClientServerSendRecvTest;

// Send Request from Client in front thread and Send Response from Server in back thread
TEST_F(WSSClientServerSendRecvTest, RequestFromClientFTResponseFromServerBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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
    EXPECT_CALL(sh, OnMessageMock(Eq(ByRef(sh.s_)), _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&srv_finished, true)));
    EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
    EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in server side
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(REQUEST, sh.m_->type);
  Request recv_req = sh.m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in client side
  ASSERT_TRUE(ch.m_ != NULL);
  ASSERT_EQ(RESPONSE, ch.m_->type);
  Response resp = ch.m_->as<Response>();
  ASSERT_EQ(req.msgid, resp.msgid);
  ASSERT_EQ(req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Server in front thread and Send Response from Client in back thread
TEST_F(WSSClientServerSendRecvTest, RequestFromServerFTResponseFromClientBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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

  EXPECT_CALL(sh, OnMessageMock(Eq(ByRef(sh.s_)), _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnMessageMock(cs, _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&cli_finished, true)));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(sh.s_);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in client side
  ASSERT_TRUE(ch.m_ != NULL);
  ASSERT_EQ(REQUEST, ch.m_->type);
  Request recv_req = ch.m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in server side
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(RESPONSE, sh.m_->type);
  Response resp = sh.m_->as<Response>();
  ASSERT_EQ(req.msgid, resp.msgid);
  ASSERT_EQ(req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Client in back thread and Send Response from Server in back thread
TEST_F(WSSClientServerSendRecvTest, RequestFromClientBTResponseFromServerBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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
    EXPECT_CALL(sh, OnMessageMock(Eq(ByRef(sh.s_)), _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&srv_finished, true)));
    EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(WithArg<0>(SendRequest()));
    EXPECT_CALL(ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
    EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(REQUEST, sh.m_->type);
  Request recv_req = sh.m_->as<Request>();
  ASSERT_TRUE(ch.m_ != NULL);
  ASSERT_EQ(RESPONSE, ch.m_->type);
  Response resp = ch.m_->as<Response>();
  ASSERT_EQ(recv_req.msgid, resp.msgid);
  ASSERT_EQ(recv_req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Server in back thread and Send Response from Client in back thread
TEST_F(WSSClientServerSendRecvTest, RequestFromServerBTResponseFromClientBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(WithArg<0>(SendRequest()));
    EXPECT_CALL(sh, OnMessageMock(Eq(ByRef(sh.s_)), _)).WillOnce(Assign(&srv_finished, true));
    EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnMessageMock(cs, _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&cli_finished, true)));
    EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(ch.m_ != NULL);
  ASSERT_EQ(REQUEST, ch.m_->type);
  Request recv_req = ch.m_->as<Request>();
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(RESPONSE, sh.m_->type);
  Response resp = sh.m_->as<Response>();
  ASSERT_EQ(recv_req.msgid, resp.msgid);
  ASSERT_EQ(recv_req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Client in front thread and not Send Response from Server(Timeout)
TEST_F(WSSClientServerSendRecvTest, RequestFromClientFTNotResponseFromServer) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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

  EXPECT_CALL(sh, OnMessageMock(sh.s_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnErrorMock(cs, _, Error(LNR_ETIMEDOUT))).WillOnce(Assign(&cli_finished, true));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(cs, 1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in server side
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(REQUEST, sh.m_->type);
  Request recv_req = sh.m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in client side
  ASSERT_TRUE(ch.err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch.err_m_->type);
  Request err_req = ch.err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
}

// Send Request from Server in front thread and not Send Response from Client(Timeout)
TEST_F(WSSClientServerSendRecvTest, RequestFromServerFTNotResponseFromClient) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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

  EXPECT_CALL(ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
  EXPECT_CALL(sh, OnErrorMock(Eq(ByRef(sh.s_)), _, Error(LNR_ETIMEDOUT))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(sh.s_, 1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in client side
  ASSERT_TRUE(ch.m_ != NULL);
  ASSERT_EQ(REQUEST, ch.m_->type);
  Request recv_req = ch.m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in server side
  ASSERT_TRUE(sh.err_m_ != NULL);
  ASSERT_EQ(REQUEST, sh.err_m_->type);
  Request err_req = sh.err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
}

// Cancel to Send Request
TEST_F(WSSClientServerSendRecvTest, CancelRequestFromClientFT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;

  // to be CONNECTING state
  WSSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(ch, OnErrorMock(cs, _, Error(LNR_ECANCELED)));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(DoAll(Assign(&cli_finished, true), Assign(&srv_finished, true)));
  }

  Request req(std::string(METHOD_NAME), Params());
  Error e = cs.Connect(); // connecting
  ASSERT_EQ(LNR_OK, e.Code());
  e = req.Send(cs); // queing request
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect(); // occur discarding request
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in client side
  ASSERT_TRUE(ch.err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch.err_m_->type);
  Request err_req = ch.err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
  ASSERT_EQ(req.method, err_req.method);
  ASSERT_EQ(req.params, err_req.params);
}

// Cancel to Send Request with Timeout
TEST_F(WSSClientServerSendRecvTest, CancelRequestWithTimeoutFromClientFT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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

  EXPECT_CALL(sh, OnConnectMock(_)).Times(::testing::AtLeast(0));
  EXPECT_CALL(sh, OnMessageMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(_)).Times(::testing::AtLeast(0));
    // occur ECANCELED or ETIMEDOUT only once
    EXPECT_CALL(ch, OnErrorMock(cs, _, _)).Times(1);
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(DoAll(Assign(&cli_finished, true), Assign(&srv_finished, true)));
  }

  Request req(std::string(METHOD_NAME), Params());
  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = req.Send(cs, 1);
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs.Disconnect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in client side
  ASSERT_TRUE(ch.err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch.err_m_->type);
  Request err_req = ch.err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
  ASSERT_EQ(req.method, err_req.method);
  ASSERT_EQ(req.params, err_req.params);
}

// Cancel to Send Request(Disconnected by peer)
TEST_F(WSSClientServerSendRecvTest, CancelRequestWithDisconnectedFromClientFT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnErrorMock(cs, _, Error(LNR_ECANCELED)));
    EXPECT_CALL(ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET))).WillOnce(Assign(&cli_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_));
    EXPECT_CALL(sh, OnMessageMock(Eq(ByRef(sh.s_)), _)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh, OnDisconnectMock(Eq(ByRef(sh.s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }

  Request req(std::string(METHOD_NAME), Params());
  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = req.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in server side
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(REQUEST, sh.m_->type);
  Request recv_req = sh.m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  ASSERT_EQ(req.method, recv_req.method);
  ASSERT_EQ(req.params, recv_req.params);
  // check message in client side
  ASSERT_TRUE(ch.err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch.err_m_->type);
  Request err_req = ch.err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
  ASSERT_EQ(req.method, err_req.method);
  ASSERT_EQ(req.params, err_req.params);
}

// Send Notify from Client in front thread
TEST_F(WSSClientServerSendRecvTest, NotifyFromClientFT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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

  EXPECT_CALL(sh, OnMessageMock(Eq(ByRef(sh.s_)), _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Notify notif(std::string(METHOD_NAME), Params());
  e = notif.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in server side
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(NOTIFY, sh.m_->type);
  Notify recv_notif = sh.m_->as<Notify>();
  ASSERT_EQ(notif.method, recv_notif.method);
  ASSERT_EQ(notif.params, recv_notif.params);
}

// Send Notify from Server in front thread
TEST_F(WSSClientServerSendRecvTest, NotifyFromServerFT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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

  EXPECT_CALL(ch, OnMessageMock(cs, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Notify notif(std::string(METHOD_NAME), Params());
  e = notif.Send(sh.s_);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in client side
  ASSERT_TRUE(ch.m_ != NULL);
  ASSERT_EQ(NOTIFY, ch.m_->type);
  Notify recv_notif = ch.m_->as<Notify>();
  ASSERT_EQ(notif.method, recv_notif.method);
  ASSERT_EQ(notif.params, recv_notif.params);
}

// Send Notify from Client in back thread
TEST_F(WSSClientServerSendRecvTest, NotifyFromClientBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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
    EXPECT_CALL(sh, OnMessageMock(Eq(ByRef(sh.s_)), _)).WillOnce(Assign(&srv_finished, true));
    EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(SendNotify()), Assign(&cli_finished, true)));
    EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(sh.m_ != NULL);
  ASSERT_EQ(NOTIFY, sh.m_->type);
  Notify recv_notif = sh.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  linear::type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Server in back thread
TEST_F(WSSClientServerSendRecvTest, NotifyFromServerBT) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
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
    EXPECT_CALL(sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(SendNotify()), Assign(&srv_finished, true)));
    EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs));
    EXPECT_CALL(ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
    EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(ch.m_ != NULL);
  ASSERT_EQ(NOTIFY, ch.m_->type);
  Notify recv_notif = ch.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  linear::type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Server to Specific Group
TEST_F(WSSClientServerSendRecvTest, NotifyFromServerToGroup) {
  MockHandler ch1, ch2, ch3;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl1(ch1, WSRequestContext(), context), cl2(ch2, WSRequestContext(), context), cl3(ch3, WSRequestContext(), context);
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

  WSSSocket cs1 = cl1.CreateSocket(TEST_ADDR, TEST_PORT);
  WSSSocket cs2 = cl2.CreateSocket(TEST_ADDR, TEST_PORT);
  WSSSocket cs3 = cl3.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(sh, OnConnectMock(_)).Times(3)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()))
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group(), WithArg<0>(LeaveFromGroup())))
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()));
    EXPECT_CALL(sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(1)).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch1, OnConnectMock(cs1));
    EXPECT_CALL(ch1, OnMessageMock(cs1, _)).Times(3);
    EXPECT_CALL(ch1, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch2, OnConnectMock(cs2));
    EXPECT_CALL(ch2, OnMessageMock(cs2, _)).Times(1);
    EXPECT_CALL(ch2, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(ch3, OnConnectMock(cs3));
    EXPECT_CALL(ch3, OnMessageMock(cs3, _)).Times(1).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(ch3, OnDisconnectMock(cs3, _)).WillOnce(Assign(&cli_finished, true));
  }

  e = cs1.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs2.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs3.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(ch1.m_ != NULL);
  ASSERT_EQ(NOTIFY, ch1.m_->type);
  Notify recv_notif = ch1.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  linear::type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_TRUE(ch2.m_ != NULL);
  ASSERT_EQ(NOTIFY, ch2.m_->type);
  recv_notif = ch2.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_EQ(NOTIFY, ch3.m_->type);
  ASSERT_TRUE(ch3.m_ != NULL);
  recv_notif = ch3.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Client to Specific Group
TEST_F(WSSClientServerSendRecvTest, NotifyFromClientToGroup) {
  MockHandler ch;
  SSLContext context(SSLContext::TLSv1_1);
  context.SetCertificate(std::string(CLIENT_CERT));
  context.SetPrivateKey(std::string(CLIENT_PKEY));
  context.SetCAFile(std::string(CA_CERT));
  context.SetCiphers(std::string(CIPHER_LIST));
  context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSClient cl(ch, WSRequestContext(), context);;
  MockHandler sh1, sh2, sh3;
  SSLContext server_context(SSLContext::TLSv1_1);
  server_context.SetCertificate(std::string(SERVER_CERT));
  server_context.SetPrivateKey(std::string(SERVER_PKEY));
  server_context.SetCAFile(std::string(CA_CERT));
  server_context.SetCiphers(std::string(CIPHER_LIST));
  server_context.SetVerifyMode(SSLContext::VERIFY_PEER);
  WSSServer sv1(sh1, server_context), sv2(sh2, server_context), sv3(sh3, server_context);

  Error e = sv1.Start(TEST_ADDR, TEST_PORT);
  ASSERT_EQ(LNR_OK, e.Code());
  e = sv2.Start(TEST_ADDR, TEST_PORT2);
  ASSERT_EQ(LNR_OK, e.Code());
  e = sv3.Start(TEST_ADDR, TEST_PORT3);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSSocket cs1 = cl.CreateSocket(TEST_ADDR, TEST_PORT);
  WSSSocket cs2 = cl.CreateSocket(TEST_ADDR, TEST_PORT2);
  WSSSocket cs3 = cl.CreateSocket(TEST_ADDR, TEST_PORT3);

  {
    InSequence dummy;
    EXPECT_CALL(ch, OnConnectMock(cs1)).Times(1)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()));
    EXPECT_CALL(ch, OnConnectMock(cs2)).Times(1)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group(), WithArg<0>(LeaveFromGroup())));
    EXPECT_CALL(ch, OnConnectMock(cs3)).Times(1)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()));
    EXPECT_CALL(ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(1)).WillOnce(Assign(&cli_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(sh1, OnConnectMock(_));
    EXPECT_CALL(sh1, OnMessageMock(Eq(ByRef(sh1.s_)), _)).Times(3);
    EXPECT_CALL(sh1, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(sh2, OnConnectMock(_)).Times(1);
    EXPECT_CALL(sh2, OnMessageMock(Eq(ByRef(sh2.s_)), _)).Times(1);
    EXPECT_CALL(sh2, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(sh3, OnConnectMock(_)).Times(1);
    EXPECT_CALL(sh3, OnMessageMock(Eq(ByRef(sh3.s_)), _)).Times(1).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(sh3, OnDisconnectMock(Eq(ByRef(sh3.s_)), _)).WillOnce(Assign(&srv_finished, true));;
  }

  e = cs1.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs2.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = cs3.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(sh1.m_ != NULL);
  ASSERT_EQ(NOTIFY, sh1.m_->type);
  Notify recv_notif = sh1.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  linear::type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_TRUE(sh2.m_ != NULL);
  ASSERT_EQ(NOTIFY, sh2.m_->type);
  recv_notif = sh2.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_TRUE(sh3.m_ != NULL);
  ASSERT_EQ(NOTIFY, sh3.m_->type);
  recv_notif = sh3.m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
}

TEST_F(WSSClientServerSendRecvTest, ZeroLengthPacket) {
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

  EXPECT_CALL(sh, OnConnectMock(_));
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  struct sockaddr_in s;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  s.sin_family = AF_INET;
  s.sin_port = htons(TEST_PORT);
  s.sin_addr.s_addr = inet_addr(TEST_ADDR);
  int ret = connect(fd, (struct sockaddr *)&s, sizeof(s));
  ASSERT_EQ(0, ret);

  SSL_CTX* ctx = SSL_CTX_new(TLSv1_1_client_method());
  SSL* ssl = SSL_new(ctx);
  SSL_set_fd(ssl, fd);
  SSL_connect(ssl);

#define HS "GET /linear HTTP/1.1\r\nConnection: upgrade\r\nHost: 127.0.0.1:37800\r\nOrigin: http://127.0.0.1:37800\r\nSec-WebSocket-Key: BJjVLqFG70hvGQZVBfJvAw==\r\nSec-WebSocket-Version: 13\r\nUpgrade: websocket\r\n\r\n"

  ssize_t siz = SSL_write(ssl, HS, strlen(HS));

  const char malformed[] = {(char)0x82, (char)0x80,
                            (char)0x00, (char)0x00, (char)0x00, (char)0x00};
  siz = SSL_write(ssl, malformed, sizeof(malformed));
  ASSERT_EQ(sizeof(malformed), (size_t)siz);
  msleep(1000);
  SSL_free(ssl);
  close(fd);
  WAIT_TO_FINISH_CALLBACK();
  SSL_CTX_free(ctx);
}

// Recv malformed packet issue #2790
TEST_F(WSSClientServerSendRecvTest, MalformedPacket) {
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

  EXPECT_CALL(sh, OnConnectMock(_)).Times(1);
  EXPECT_CALL(sh, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  struct sockaddr_in s;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  s.sin_family = AF_INET;
  s.sin_port = htons(TEST_PORT);
  s.sin_addr.s_addr = inet_addr(TEST_ADDR);
  int ret = connect(fd, (struct sockaddr *)&s, sizeof(s));
  ASSERT_EQ(0, ret);

  SSL_CTX* ctx = SSL_CTX_new(TLSv1_1_client_method());
  SSL* ssl = SSL_new(ctx);
  SSL_set_fd(ssl, fd);
  SSL_connect(ssl);

#define HS "GET /linear HTTP/1.1\r\nConnection: upgrade\r\nHost: 127.0.0.1:37800\r\nOrigin: http://127.0.0.1:37800\r\nSec-WebSocket-Key: BJjVLqFG70hvGQZVBfJvAw==\r\nSec-WebSocket-Version: 13\r\nUpgrade: websocket\r\n\r\n"

  ssize_t siz = SSL_write(ssl, HS, strlen(HS));

  const char malformed[] = {(char)0x82, (char)0x85, (char)0x00, (char)0x00, (char)0x00, (char)0x00,
                            (char)0xdd,
                            (char)0x0a, (char)0xaa, (char)0xaa, (char)0xab};
  siz = SSL_write(ssl, malformed, sizeof(malformed));
  ASSERT_EQ(sizeof(malformed), (size_t)siz);
  msleep(1000);
  if (sizeof(size_t) == 4) {
    WAIT_TO_FINISH_CALLBACK();
  }
  SSL_free(ssl);
  close(fd);
  if (sizeof(size_t) != 4) {
    WAIT_TO_FINISH_CALLBACK();
  }
  SSL_CTX_free(ctx);
}
