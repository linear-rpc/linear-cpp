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

typedef LinearTest WSClientServerSendRecvTest;

// Send Request from Client in front thread and Send Response from Server in back thread
TEST_F(WSClientServerSendRecvTest, RequestFromClientFTResponseFromServerBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_));
    EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&srv_finished, true)));
    EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
    EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, RequestFromServerFTResponseFromClientBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnMessageMock(cs, _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&cli_finished, true)));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(sh->s_);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, RequestFromClientBTResponseFromServerBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_));
    EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&srv_finished, true)));
    EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(WithArg<0>(SendRequest()));
    EXPECT_CALL(*ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
    EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, RequestFromServerBTResponseFromClientBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(WithArg<0>(SendRequest()));
    EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _)).WillOnce(Assign(&srv_finished, true));
    EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnMessageMock(cs, _)).WillOnce(DoAll(WithArgs<0, 1>(SendResponse()), Assign(&cli_finished, true)));
    EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, RequestFromClientFTNotResponseFromServer) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*sh, OnMessageMock(sh->s_, _)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnErrorMock(cs, _, Error(LNR_ETIMEDOUT))).WillOnce(Assign(&cli_finished, true));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(cs, 1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, RequestFromServerFTNotResponseFromClient) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
  EXPECT_CALL(*sh, OnErrorMock(Eq(ByRef(sh->s_)), _, Error(LNR_ETIMEDOUT))).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Params msg;
  Request req(std::string(METHOD_NAME), msg);
  e = req.Send(sh->s_, 1);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, CancelRequestFromClientFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);

  // to be CONNECTING state
  WSSocket cs = cl.CreateSocket(TEST_ADDR_4_TIMEOUT, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnErrorMock(cs, _, Error(LNR_ECANCELED)));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(DoAll(Assign(&cli_finished, true), Assign(&srv_finished, true)));
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
  ASSERT_TRUE(ch->err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch->err_m_->type);
  Request err_req = ch->err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
  ASSERT_EQ(req.method, err_req.method);
  ASSERT_EQ(req.params, err_req.params);
}

// Cancel to Send Request with Timeout
TEST_F(WSClientServerSendRecvTest, CancelRequestWithTimeoutFromClientFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*sh, OnMessageMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(_)).Times(::testing::AtLeast(0));
    // occur ECANCELED or ETIMEDOUT only once
    EXPECT_CALL(*ch, OnErrorMock(cs, _, _)).Times(1);
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_OK))).WillOnce(DoAll(Assign(&cli_finished, true), Assign(&srv_finished, true)));
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
  ASSERT_TRUE(ch->err_m_ != NULL);
  ASSERT_EQ(REQUEST, ch->err_m_->type);
  Request err_req = ch->err_m_->as<Request>();
  ASSERT_EQ(req.msgid, err_req.msgid);
  ASSERT_EQ(req.method, err_req.method);
  ASSERT_EQ(req.params, err_req.params);
}

// Cancel to Send Request(Disconnected by peer)
TEST_F(WSClientServerSendRecvTest, CancelRequestWithDisconnectedFromClientFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnErrorMock(cs, _, Error(LNR_ECANCELED)));
    EXPECT_CALL(*ch, OnDisconnectMock(cs, Error(LNR_ECONNRESET))).WillOnce(Assign(&cli_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_));
    EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh, OnDisconnectMock(Eq(ByRef(sh->s_)), Error(LNR_OK))).WillOnce(Assign(&srv_finished, true));
  }

  Request req(std::string(METHOD_NAME), Params());
  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  e = req.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, NotifyFromClientFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Notify notif(std::string(METHOD_NAME), Params());
  e = notif.Send(cs);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in server side
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(NOTIFY, sh->m_->type);
  Notify recv_notif = sh->m_->as<Notify>();
  ASSERT_EQ(notif.method, recv_notif.method);
  ASSERT_EQ(notif.params, recv_notif.params);
}

// Send Notify from Server in front thread
TEST_F(WSClientServerSendRecvTest, NotifyFromServerFT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());
  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(Assign(&srv_finished, true));
  EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(Assign(&cli_finished, true));

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  EXPECT_CALL(*ch, OnMessageMock(cs, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));

  Notify notif(std::string(METHOD_NAME), Params());
  e = notif.Send(sh->s_);
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check message in client side
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(NOTIFY, ch->m_->type);
  Notify recv_notif = ch->m_->as<Notify>();
  ASSERT_EQ(notif.method, recv_notif.method);
  ASSERT_EQ(notif.params, recv_notif.params);
}

// Send Notify from Client in back thread
TEST_F(WSClientServerSendRecvTest, NotifyFromClientBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_));
    EXPECT_CALL(*sh, OnMessageMock(Eq(ByRef(sh->s_)), _)).WillOnce(Assign(&srv_finished, true));
    EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs)).WillOnce(DoAll(WithArg<0>(SendNotify()), Assign(&cli_finished, true)));
    EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(sh->m_ != NULL);
  ASSERT_EQ(NOTIFY, sh->m_->type);
  Notify recv_notif = sh->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Server in back thread
TEST_F(WSClientServerSendRecvTest, NotifyFromServerBT) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs = cl.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).WillOnce(DoAll(WithArg<0>(SendNotify()), Assign(&srv_finished, true)));
    EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs));
    EXPECT_CALL(*ch, OnMessageMock(cs, _)).WillOnce(Assign(&cli_finished, true));
    EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }

  e = cs.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

  // check messages
  ASSERT_TRUE(ch->m_ != NULL);
  ASSERT_EQ(NOTIFY, ch->m_->type);
  Notify recv_notif = ch->m_->as<Notify>();
  ASSERT_EQ(std::string(METHOD_NAME), recv_notif.method);
  type::any params = Params();
  ASSERT_EQ(params, recv_notif.params);
}

// Send Notify from Server to Specific Group
TEST_F(WSClientServerSendRecvTest, NotifyFromServerToGroup) {
  shared_ptr<MockHandler> ch1 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> ch2 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> ch3 = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl1(ch1), cl2(ch2), cl3(ch3);
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs1 = cl1.CreateSocket(TEST_ADDR, TEST_PORT);
  WSSocket cs2 = cl2.CreateSocket(TEST_ADDR, TEST_PORT);
  WSSocket cs3 = cl3.CreateSocket(TEST_ADDR, TEST_PORT);

  {
    InSequence dummy;
    EXPECT_CALL(*sh, OnConnectMock(_)).Times(3)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()))
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group(), WithArg<0>(LeaveFromGroup())))
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()));
    EXPECT_CALL(*sh, OnDisconnectMock(_, _)).Times(::testing::AtLeast(1)).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch1, OnConnectMock(cs1)).WillOnce(Assign(&cli_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch1, OnMessageMock(cs1, _)).Times(2);
    EXPECT_CALL(*ch1, OnMessageMock(cs1, _)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*ch1, OnDisconnectMock(_, _)).WillOnce(Assign(&cli_finished, true));
  }
  srv_finished = true;
  e = cs1.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  {
    InSequence dummy;
    EXPECT_CALL(*ch2, OnConnectMock(cs2)).WillOnce(Assign(&cli_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch2, OnMessageMock(cs2, _)).Times(1);
    EXPECT_CALL(*ch2, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  srv_finished = true;
  e = cs2.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  {
    InSequence dummy;
    EXPECT_CALL(*ch3, OnConnectMock(cs3));
    EXPECT_CALL(*ch3, OnMessageMock(cs3, _)).Times(1);
    EXPECT_CALL(*ch3, OnDisconnectMock(cs3, _)).Times(::testing::AtLeast(0));
  }
  e = cs3.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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
TEST_F(WSClientServerSendRecvTest, NotifyFromClientToGroup) {
  shared_ptr<MockHandler> ch = linear::shared_ptr<MockHandler>(new MockHandler());
  WSClient cl(ch);
  shared_ptr<MockHandler> sh1 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> sh2 = linear::shared_ptr<MockHandler>(new MockHandler());
  shared_ptr<MockHandler> sh3 = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv1(sh1), sv2(sh2), sv3(sh3);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv1.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());
  e = sv2.Start(TEST_ADDR, TEST_PORT2);
  ASSERT_EQ(LNR_OK, e.Code());
  e = sv3.Start(TEST_ADDR, TEST_PORT3);
  ASSERT_EQ(LNR_OK, e.Code());

  WSSocket cs1 = cl.CreateSocket(TEST_ADDR, TEST_PORT);
  WSSocket cs2 = cl.CreateSocket(TEST_ADDR, TEST_PORT2);
  WSSocket cs3 = cl.CreateSocket(TEST_ADDR, TEST_PORT3);

  {
    InSequence dummy;
    EXPECT_CALL(*sh1, OnConnectMock(_));
    EXPECT_CALL(*sh1, OnMessageMock(Eq(ByRef(sh1->s_)), _)).Times(2);
    EXPECT_CALL(*sh1, OnMessageMock(Eq(ByRef(sh1->s_)), _)).WillOnce(WithArg<0>(Disconnect()));
    EXPECT_CALL(*sh1, OnDisconnectMock(_, _)).Times(::testing::AtLeast(1)).WillOnce(Assign(&srv_finished, true));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*sh2, OnConnectMock(_)).Times(1);
    EXPECT_CALL(*sh2, OnMessageMock(Eq(ByRef(sh2->s_)), _)).Times(1);
    EXPECT_CALL(*sh2, OnDisconnectMock(_, _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*sh3, OnConnectMock(_)).Times(1);
    EXPECT_CALL(*sh3, OnMessageMock(Eq(ByRef(sh3->s_)), _)).Times(1);
    EXPECT_CALL(*sh3, OnDisconnectMock(Eq(ByRef(sh3->s_)), _)).Times(::testing::AtLeast(0));
  }
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs1)).Times(1)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group(), Assign(&cli_finished, true)));
  }
  srv_finished = true;
  e = cs1.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs2)).Times(1)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group(), WithArg<0>(LeaveFromGroup()), Assign(&cli_finished, true)));
  }
  srv_finished = true;
  e = cs2.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();
  {
    InSequence dummy;
    EXPECT_CALL(*ch, OnConnectMock(cs3)).Times(1)
      .WillOnce(DoAll(WithArg<0>(JoinToGroup()), SendNotify2Group()));
    EXPECT_CALL(*ch, OnDisconnectMock(_, _)).Times(::testing::AtLeast(1)).WillOnce(Assign(&cli_finished, true));
  }
  e = cs3.Connect();
  ASSERT_EQ(LNR_OK, e.Code());
  WAIT_TO_FINISH_CALLBACK();

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

TEST_F(WSClientServerSendRecvTest, ZeroLengthPacket) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_));
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  const char malformed[] = {(char)0x82, (char)0x80,
                            (char)0x00, (char)0x00, (char)0x00, (char)0x00};
  struct sockaddr_in s;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  s.sin_family = AF_INET;
  s.sin_port = htons(TEST_PORT);
  s.sin_addr.s_addr = inet_addr(TEST_ADDR);
  int ret = connect(fd, (struct sockaddr *)&s, sizeof(s));
  ASSERT_EQ(0, ret);

#define HS "GET /linear HTTP/1.1\r\nConnection: upgrade\r\nHost: 127.0.0.1:37800\r\nOrigin: http://127.0.0.1:37800\r\nSec-WebSocket-Key: BJjVLqFG70hvGQZVBfJvAw==\r\nSec-WebSocket-Version: 13\r\nUpgrade: websocket\r\n\r\n"
    
  ssize_t siz = write(fd, HS, strlen(HS));
  siz = write(fd, malformed, sizeof(malformed));
  ASSERT_EQ(sizeof(malformed), (size_t)siz);
  msleep(1000);
  close(fd);
  WAIT_TO_FINISH_CALLBACK();
}

// Recv malformed packet issue #2790
TEST_F(WSClientServerSendRecvTest, MalformedPacket) {
  shared_ptr<MockHandler> sh = linear::shared_ptr<MockHandler>(new MockHandler());
  WSServer sv(sh);

  Error e;
  for (int i = 0; i < 3; i++) {
    e = sv.Start(TEST_ADDR, TEST_PORT);
    if (e == linear::Error(LNR_OK)) {
      break;
    }
    sleep(1);
  }
  ASSERT_EQ(LNR_OK, e.Code());

  EXPECT_CALL(*sh, OnConnectMock(_)).Times(1);
  EXPECT_CALL(*sh, OnDisconnectMock(_, _)).WillOnce(DoAll(Assign(&srv_finished, true), Assign(&cli_finished, true)));

  const char malformed[] = {(char)0x82, (char)0x85, (char)0x00, (char)0x00, (char)0x00, (char)0x00,
                            (char)0xdd,
                            (char)0x0a, (char)0xaa, (char)0xaa, (char)0xab};
  struct sockaddr_in s;
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  s.sin_family = AF_INET;
  s.sin_port = htons(TEST_PORT);
  s.sin_addr.s_addr = inet_addr(TEST_ADDR);
  int ret = connect(fd, (struct sockaddr *)&s, sizeof(s));
  ASSERT_EQ(0, ret);

#define HS "GET /linear HTTP/1.1\r\nConnection: upgrade\r\nHost: 127.0.0.1:37800\r\nOrigin: http://127.0.0.1:37800\r\nSec-WebSocket-Key: BJjVLqFG70hvGQZVBfJvAw==\r\nSec-WebSocket-Version: 13\r\nUpgrade: websocket\r\n\r\n"

  ssize_t siz = write(fd, HS, strlen(HS));
  siz = write(fd, malformed, sizeof(malformed));
  ASSERT_EQ(sizeof(malformed), (size_t)siz);
  msleep(1000);
  if (sizeof(size_t) == 4) {
    WAIT_TO_FINISH_CALLBACK();
  }
  close(fd);
  if (sizeof(size_t) != 4) {
    WAIT_TO_FINISH_CALLBACK();
  }
}
