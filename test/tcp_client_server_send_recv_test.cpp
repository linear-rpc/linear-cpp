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

typedef LinearTest TCPClientServerSendRecvTest;

// Send Request from Client in front thread and Send Response from Server in back thread
TEST_F(TCPClientServerSendRecvTest, RequestFromClientFTResponseFromServerBT) {
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

  EXPECT_CALL(*sh, OnConnectMock(_));
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(WithArgs<0, 1>(SendResponse()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .WillOnce(WithArgs<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());

  WAIT_TESTED();

  // check message in server side
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(REQUEST, sh->m_->type);
  Request recv_req = sh->m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in client side
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(RESPONSE, ch->m_->type);
  Response resp = ch->m_->as<Response>();
  ASSERT_EQ(req.msgid, resp.msgid);
  ASSERT_EQ(req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Server in front thread and Send Response from Client in back thread
TEST_F(TCPClientServerSendRecvTest, RequestFromServerFTResponseFromClientBT) {
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
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(WithArgs<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .WillOnce(WithArgs<0, 1>(SendResponse()));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(sh->s_);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check message in client side
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(REQUEST, ch->m_->type);
  Request recv_req = ch->m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in server side
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(RESPONSE, sh->m_->type);
  Response resp = sh->m_->as<Response>();
  ASSERT_EQ(req.msgid, resp.msgid);
  ASSERT_EQ(req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Client in back thread and Send Response from Server in back thread
TEST_F(TCPClientServerSendRecvTest, RequestFromClientBTResponseFromServerBT) {
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

  EXPECT_CALL(*sh, OnConnectMock(_));
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(WithArgs<0, 1>(SendResponse()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(WithArgs<0>(SendRequest()));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .WillOnce(WithArgs<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check messages
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(REQUEST, sh->m_->type);
  Request recv_req = sh->m_->as<Request>();
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(RESPONSE, ch->m_->type);
  Response resp = ch->m_->as<Response>();
  ASSERT_EQ(recv_req.msgid, resp.msgid);
  ASSERT_EQ(recv_req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Server in back thread and Send Response from Client in back thread
TEST_F(TCPClientServerSendRecvTest, RequestFromServerBTResponseFromClientBT) {
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
    .WillOnce(WithArgs<0>(SendRequest()));
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(WithArgs<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .WillOnce(WithArgs<0, 1>(SendResponse()));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check messages
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(REQUEST, ch->m_->type);
  Request recv_req = ch->m_->as<Request>();
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(RESPONSE, sh->m_->type);
  Response resp = sh->m_->as<Response>();
  ASSERT_EQ(recv_req.msgid, resp.msgid);
  ASSERT_EQ(recv_req.params, resp.result);
  ASSERT_TRUE(resp.error.is_nil());
}

// Send Request from Client in front thread and not Send Response from Server(Timeout)
TEST_F(TCPClientServerSendRecvTest, RequestFromClientFTNotResponseFromServer) {
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
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .Times(0);
  EXPECT_CALL(*ch, OnErrorMock(cs, _, Error(LNR_ETIMEDOUT)))
    .WillOnce(WithArgs<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(cs, 1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check message in server side
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(REQUEST, sh->m_->type);
  Request recv_req = sh->m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in client side
  ASSERT_TRUE(ch->err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch->err_m_->type);
  Request err_req = ch->err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
}

// Send Request from Server in front thread and not Send Response from Client(Timeout)
TEST_F(TCPClientServerSendRecvTest, RequestFromServerFTNotResponseFromClient) {
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
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .Times(0);
  EXPECT_CALL(*sh, OnErrorMock(Eq(ByRef(sh->s_)), _, Error(LNR_ETIMEDOUT)))
    .WillOnce(WithArgs<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(sh->s_, 1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check message in client side
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(REQUEST, ch->m_->type);
  Request recv_req = ch->m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  // check message in server side
  ASSERT_TRUE(sh->err_m_ != NULL);
  ASSERT_EQ(REQUEST, sh->err_m_->type);
  Request err_req = sh->err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
}

// Cancel to Send Request
TEST_F(TCPClientServerSendRecvTest, CancelRequestFromClientFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  // to become CONNECTING state
  TCPSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  EXPECT_CALL(*ch, OnConnectMock(cs))
    .Times(0);
  EXPECT_CALL(*ch, OnErrorMock(cs, _, Error(LNR_ECANCELED)));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK)))
    .WillOnce(Assign(&cli_tested, true));

  Error e = cs.Connect(); // connecting
  ASSERT_EQ(LNR_OK, e.Code());

  Request req(std::string(METHOD_NAME), Params());
  e = req.Send(cs); // queing request
  ASSERT_EQ(LNR_OK, e.Code());

  e = cs.Disconnect(); // occur discarding request
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CLI_TESTED();

  // check message in client side
  ASSERT_TRUE(ch->err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch->err_m_->type);
  Request err_req = ch->err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
  ASSERT_EQ(req.method, err_req.method);
  ASSERT_EQ(req.params, err_req.params);
}

// Cancel to Send Request(Disconnected by peer)
TEST_F(TCPClientServerSendRecvTest, CancelRequestWithDisconnectedFromClientFT) {
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

  EXPECT_CALL(*sh, OnConnectMock(_));
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK)))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs));
  EXPECT_CALL(*ch, OnErrorMock(cs, _, Error(LNR_ECANCELED)));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_EOF)))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());

  Request req(std::string(METHOD_NAME), Params());
  e = req.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check message in server side
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(REQUEST, sh->m_->type);
  Request recv_req = sh->m_->as<Request>();
  ASSERT_EQ(req.msgid, recv_req.msgid);
  ASSERT_EQ(req.method, recv_req.method);
  ASSERT_EQ(req.params, recv_req.params);
  // check message in client side
  ASSERT_TRUE(ch->err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch->err_m_->type);
  Request err_req = ch->err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
  ASSERT_EQ(req.method, err_req.method);
  ASSERT_EQ(req.params, err_req.params);
}

// Send Notify from Client in front thread
TEST_F(TCPClientServerSendRecvTest, NotifyFromClientFT) {
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

  EXPECT_CALL(*sh, OnConnectMock(_));
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  Notify notif(std::string(METHOD_NAME), Params());
  e = notif.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check message in server side
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(NOTIFY, sh->m_->type);
  Notify recv_notif = sh->m_->as<Notify>();
  ASSERT_EQ(notif.method, recv_notif.method);
  ASSERT_EQ(notif.params, recv_notif.params);
}

// Send Notify from Server in front thread
TEST_F(TCPClientServerSendRecvTest, NotifyFromServerFT) {
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
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();
  Notify notif(std::string(METHOD_NAME), Params());
  e = notif.Send(sh->s_);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check message in client side
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(NOTIFY, ch->m_->type);
  Notify recv_notif = ch->m_->as<Notify>();
  ASSERT_EQ(notif.method, recv_notif.method);
  ASSERT_EQ(notif.params, recv_notif.params);
}

// Send Notify from Client in back thread
TEST_F(TCPClientServerSendRecvTest, NotifyFromClientBT) {
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

  EXPECT_CALL(*sh, OnConnectMock(_));
  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(WithArg<0>(SendNotify()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check messages
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(NOTIFY, sh->m_->type);
  Notify recv_notif = sh->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Server in back thread
TEST_F(TCPClientServerSendRecvTest, NotifyFromServerBT) {
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
    .WillOnce(WithArg<0>(SendNotify()));
  EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), _))
    .WillOnce(Assign(&srv_tested, true));
  EXPECT_CALL(*ch, OnConnectMock(cs));
  EXPECT_CALL(*ch, OnMessageMock(cs, _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(cs, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();

  // check messages
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(NOTIFY, ch->m_->type);
  Notify recv_notif = ch->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Server to Specific Group
TEST_F(TCPClientServerSendRecvTest, NotifyFromServerToGroup) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<MockHandler> ch1 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> ch2 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> ch3 = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl1(ch1), cl2(ch2), cl3(ch3);
  TCPSocket cs1 = cl1.CreateSocket(TEST_ADDR, TEST_PORT);
  TCPSocket cs2 = cl2.CreateSocket(TEST_ADDR, TEST_PORT);
  TCPSocket cs3 = cl3.CreateSocket(TEST_ADDR, TEST_PORT);

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
    .WillOnce(DoAll(Assign(&srv_connected, true), WithArg<0>(JoinToGroup()), SendNotify2Group()))
    .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group(), WithArg<0>(LeaveFromGroup())))
    .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .Times(::testing::AtLeast(2))
    .WillOnce(::testing::Return())
    .WillOnce(DoAll(Assign(&srv_tested, true),
		    Assign(&srv_connected, false)));

  EXPECT_CALL(*ch1, OnConnectMock(cs1))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch1, OnMessageMock(cs1, _))
    .WillOnce(::testing::Return())
    .WillOnce(::testing::Return())
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch1, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_tested, true));

  e = cs1.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  EXPECT_CALL(*ch2, OnConnectMock(cs2))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch2, OnMessageMock(cs2, _))
    .Times(1);
  EXPECT_CALL(*ch2, OnDisconnectMock(_, _))
    .Times(::testing::AtLeast(0));

  cli_connected = false;
  e = cs2.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  EXPECT_CALL(*ch3, OnConnectMock(cs3));
  EXPECT_CALL(*ch3, OnMessageMock(cs3, _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch3, OnDisconnectMock(cs3, _))
    .WillOnce(Assign(&cli_connected, false));
  
  e = cs3.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
  WAIT_DISCONNECTED();

  // check messages
  ASSERT_TRUE(ch1->m_ != NULL);
  ASSERT_EQ(NOTIFY, ch1->m_->type);
  Notify recv_notif = ch1->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_TRUE(ch2->m_ != NULL);
  ASSERT_EQ(NOTIFY, ch2->m_->type);
  recv_notif = ch2->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_EQ(NOTIFY, ch3->m_->type);
  ASSERT_TRUE(ch3->m_ != NULL);
  recv_notif = ch3->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Client to Specific Group
TEST_F(TCPClientServerSendRecvTest, NotifyFromClientToGroup) {
  shared_ptr<MockHandler> sh1 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> sh2 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> sh3 = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv1(sh1), sv2(sh2), sv3(sh3);
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPClient cl(ch);
  TCPSocket cs1 = cl.CreateSocket(TEST_ADDR, TEST_PORT);
  TCPSocket cs2 = cl.CreateSocket(TEST_ADDR, TEST_PORT2);
  TCPSocket cs3 = cl.CreateSocket(TEST_ADDR, TEST_PORT3);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv1.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    msleep(100);
  }
  ASSERT_EQ(LNR_OK, e.Code());
  e = sv2.Start(TEST_ADDR, TEST_PORT2);
  ASSERT_EQ(LNR_OK, e.Code());
  e = sv3.Start(TEST_ADDR, TEST_PORT3);
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*ch, OnConnectMock(_))
    .WillOnce(DoAll(Assign(&cli_connected, true), WithArg<0>(JoinToGroup()), SendNotify2Group()))
    .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group(), WithArg<0>(LeaveFromGroup())))
    .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .Times(::testing::AtLeast(2))
    .WillOnce(::testing::Return())
    .WillOnce(DoAll(Assign(&cli_tested, true),
		    Assign(&cli_connected, false)));

  EXPECT_CALL(*sh1, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh1, OnMessageMock(_, _))
    .WillOnce(::testing::Return())
    .WillOnce(::testing::Return())
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh1, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));

  e = cs1.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();

  EXPECT_CALL(*sh2, OnConnectMock(_))
    .WillOnce(Assign(&srv_connected, true));
  EXPECT_CALL(*sh2, OnMessageMock(_, _))
    .Times(1);
  EXPECT_CALL(*sh2, OnDisconnectMock(_, _))
    .Times(::testing::AtLeast(0));

  srv_connected = false;
  e = cs2.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();
  
  EXPECT_CALL(*sh3, OnConnectMock(_));
  EXPECT_CALL(*sh3, OnMessageMock(_, _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*sh3, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_connected, false));

  e = cs3.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TESTED();
  WAIT_DISCONNECTED();

  // check messages
  ASSERT_TRUE(sh1->m_ != NULL);
  ASSERT_EQ(NOTIFY, sh1->m_->type);
  Notify recv_notif = sh1->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_TRUE(sh2->m_ != NULL);
  ASSERT_EQ(NOTIFY, sh2->m_->type);
  recv_notif = sh2->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
  ASSERT_TRUE(sh3->m_ != NULL);
  ASSERT_EQ(NOTIFY, sh3->m_->type);
  recv_notif = sh3->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  ASSERT_EQ(params, recv_notif.params);
}

#ifndef _WIN32
// Recv malformed packet, issue #149: https://github.com/msgpack/msgpack-c/issues/149
TEST_F(TCPClientServerSendRecvTest, MalformedPacket) {
  if (sizeof(size_t) != 4) {
    LINEAR_LOG(LOG_DEBUG, "sizeof(size_t) != 4, so ignore this test");
    return;
  }
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);

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
    .Times(1);
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));

  const char malformed[] = {(char)0xdd,
                            (char)0x0a, (char)0xaa, (char)0xaa, (char)0xab};
  struct sockaddr_in s;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  s.sin_family = AF_INET;
  s.sin_port = htons(TEST_PORT);
  s.sin_addr.s_addr = inet_addr(TEST_ADDR);
  int ret = connect(fd, (struct sockaddr *)&s, sizeof(s));
  ASSERT_EQ(0, ret);
  ssize_t siz = write(fd, malformed, sizeof(malformed));
  ASSERT_EQ(sizeof(malformed), (size_t)siz);
  WAIT_SRV_TESTED();
  close(fd);
}
#endif

// Overflow SendBuffer
TEST_F(TCPClientServerSendRecvTest, SendBuffer) {
  linear::EventLoop loop;
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  TCPServer sv(sh);
  shared_ptr<BlockMockHandler> ch = linear::shared_ptr<BlockMockHandler>(new BlockMockHandler());
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
    .WillOnce(DoAll(Assign(&srv_connected, true), WithArg<0>(MultiSendNotify(2, 10000))));
  EXPECT_CALL(*sh, OnErrorMock(_, _, _))
    .WillRepeatedly(DoAll(WithArg<2>(CheckEbusy()), Assign(&srv_tested, true)));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_connected, false));
  EXPECT_CALL(*ch, OnConnectMock(cs))
    .WillOnce(Assign(&cli_connected, true));
  EXPECT_CALL(*ch, OnMessageMock(_, _))
    .WillOnce(WithArg<0>(Disconnect()));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _))
    .WillOnce(Assign(&cli_connected, false));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());

  WAIT_CONNECTED();
  WAIT_SRV_TESTED();
  ch->do_block = false;
  WAIT_DISCONNECTED();
}

// Must not Overflow SendBuffer
TEST_F(TCPClientServerSendRecvTest, NotOverflowSendBuffer) {
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
    .WillOnce(DoAll(Assign(&srv_connected, true), WithArg<0>(MultiSendNotify(10, 0))));
  EXPECT_CALL(*sh, OnErrorMock(_, _, _))
    .Times(0);
  EXPECT_CALL(*sh, OnDisconnectMock(_, _))
    .WillOnce(Assign(&srv_tested, true));
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs))
      .WillOnce(Assign(&cli_connected, true));
    EXPECT_CALL(*ch, OnMessageMock(_, _))
      .Times(9);
    EXPECT_CALL(*ch, OnMessageMock(_, _))
      .WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*ch, OnDisconnectMock(_, _))
      .WillOnce(Assign(&cli_tested, true));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_CONNECTED();
  WAIT_TESTED();
}
