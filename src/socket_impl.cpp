#include <sstream>

#include "linear/ws_socket.h"

#include "ws_socket_impl.h"
#include "handler_delegate.h"

#ifdef WITH_SSL
# include "linear/wss_socket.h"
# include "wss_socket_impl.h"
#endif

using namespace linear::log;

namespace linear {

static linear::mutex g_id_mutex;

static int Id() {
  lock_guard<mutex> lock(g_id_mutex);
  static int id = 0;
  return id++;
}

static std::string GetTypeString(Socket::Type type) {
  std::string proto("NIL");
  switch(type) {
  case Socket::TCP:
    proto = "TCP";
    break;
  case Socket::SSL:
    proto = "SSL";
    break;
  case Socket::WS:
    proto = "WS";
    break;
  case Socket::WSS:
    proto = "WSS";
    break;
  case Socket::NIL:
  default:
    break;
  }
  return proto;  
}

// Client Socket
SocketImpl::SocketImpl(const std::string& host, int port,
                       const linear::shared_ptr<linear::EventLoopImpl>& loop,
                       const HandlerDelegate& delegate,
                       Socket::Type type)
  : state_(Socket::DISCONNECTED),
    stream_(NULL), ev_(NULL), peer_(Addrinfo(host, port)), loop_(loop), type_(type), id_(Id()),
    connectable_(true), handshaking_(false), last_error_(LNR_OK), observer_(delegate.GetObserver()),
    connect_timeout_(0),
    max_buffer_size_(Socket::DEFAULT_MAX_BUFFER_SIZE) {
  LINEAR_LOG(LOG_DEBUG, "socket(id = %d, type = %s, connectable) is created",
             id_, GetTypeString(type_).c_str());
}

// Server Socket
SocketImpl::SocketImpl(tv_stream_t* stream,
                       const linear::shared_ptr<linear::EventLoopImpl>& loop,
                       const HandlerDelegate& delegate,
                       Socket::Type type)
  : stream_(stream), ev_(NULL), loop_(loop), type_(type), id_(Id()),
    connectable_(false), last_error_(LNR_OK), observer_(delegate.GetObserver()),
    max_buffer_size_(Socket::DEFAULT_MAX_BUFFER_SIZE) {
  LINEAR_LOG(LOG_DEBUG, "socket(id = %d, type = %s, not connectable) is created",
             id_, GetTypeString(type_).c_str());
  if (type == Socket::WS) {
    handshaking_ = true;
    state_ = Socket::CONNECTING;
    reinterpret_cast<tv_ws_t*>(stream_)->handshake_complete_cb = EventLoopImpl::OnAcceptComplete;

#ifdef WITH_SSL
  } else if (type == Socket::WSS) {
    handshaking_ = true;
    state_ = Socket::CONNECTING;
    reinterpret_cast<tv_wss_t*>(stream_)->handshake_complete_cb = EventLoopImpl::OnAcceptComplete;
#endif

  } else {
    handshaking_ = false;
    state_ = Socket::CONNECTED;
  }
  struct sockaddr_storage ss;
  int len = sizeof(struct sockaddr_storage);
  int ret = tv_getsockname(stream_, reinterpret_cast<struct sockaddr*>(&ss), &len);
  if (ret == 0) {
    self_ = Addrinfo(reinterpret_cast<struct sockaddr*>(&ss));
  }
  ret = tv_getpeername(stream_, reinterpret_cast<struct sockaddr*>(&ss), &len);
  if (ret == 0) {
    peer_ = Addrinfo(reinterpret_cast<struct sockaddr*>(&ss));
  } else {
    LINEAR_LOG(LOG_WARN, "fail to get peerinfo(id = %d) (may disconnected by peer): %s",
               id_, tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
  }
  LINEAR_LOG(LOG_DEBUG, "incoming peer(id = %d): %s:%d <-- %s --- %s:%d",
             id_,
             self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
             peer_.addr.c_str(), peer_.port);
}

SocketImpl::~SocketImpl() {
  Disconnect(false);
  LINEAR_LOG(LOG_DEBUG, "socket(id = %d, type = %s, %s) is destroyed",
             id_, GetTypeString(type_).c_str(),
             (connectable_ ? "connectable" : "not connectable"));
}

void SocketImpl::SetMaxBufferSize(size_t limit) {
  max_buffer_size_ = limit;
}

Error SocketImpl::Connect(unsigned int timeout, EventLoopImpl::SocketEvent* ev) {
  lock_guard<mutex> state_lock(state_mutex_);
  if (!connectable_) {
    LINEAR_LOG(LOG_WARN, "this socket(id = %d) is not connectable", id_);
    return Error(LNR_EINVAL);
  }
  if (state_ == Socket::CONNECTING || state_ == Socket::CONNECTED) {
    LINEAR_LOG(LOG_INFO, "this socket(id = %d) is %s",
               id_,
               (state_ == Socket::CONNECTING) ? "connecting now" : "already connected");
    return Error(LNR_EALREADY);
  } else if (state_ == Socket::DISCONNECTING) {
    LINEAR_LOG(LOG_WARN, "this socket(id = %d) is disconnecting now.plz call later.", id_);
    return Error(LNR_EBUSY);
  }
  LINEAR_LOG(LOG_DEBUG, "try to connect(id = %d): --- %s --> %s:%d",
             id_,
             GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
  ev_ = ev;
  Error err;
  shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock();
  shared_ptr<SocketImpl> socket = ev_->socket.lock();
  if (observer && socket) {
    bool is_locked = observer->TryLock();
    HandlerDelegate* delegate = observer->GetSubject();
    if (delegate) {
      err = delegate->Retain(socket);
      if (err != Error(LNR_OK)) {
        LINEAR_LOG(LOG_ERR, "fail to connect(id = %d): %s", id_, err.Message().c_str());
        if (is_locked) {
          observer->Unlock();
        }
        return err;
      }
    }
    if (is_locked) {
      observer->Unlock();
    }
  }
  self_ = Addrinfo(); // reset self info
  err = Connect();
  if (err == Error(LNR_OK)) {
    state_ = Socket::CONNECTING;
    if (timeout > 0) {
      connect_timeout_ = timeout;
      connect_timer_.Start(EventLoopImpl::OnConnectTimeout, connect_timeout_, ev_);
    } else {
      connect_timeout_ = 0;
    }
  } else {
    LINEAR_LOG(LOG_ERR, "fail to connect(id = %d): %s", id_, err.Message().c_str());
    if (observer && socket) {
      bool is_locked = observer->TryLock();
      HandlerDelegate* delegate = observer->GetSubject();
      if (delegate) {
        delegate->Release(socket);
      }
      if (is_locked) {
        observer->Unlock();
      }
    }
  }
  return err;
}

Error SocketImpl::Disconnect(bool handshaking) {
  lock_guard<mutex> state_lock(state_mutex_);
  handshaking_ = handshaking;
  if (state_ == Socket::DISCONNECTING || state_ == Socket::DISCONNECTED) {
    return Error(LNR_EALREADY);
  }
  connect_timer_.Stop();
  state_ = Socket::DISCONNECTING;
  last_error_ = Error(LNR_OK);
  tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoopImpl::OnClose);
  return Error(LNR_OK);
}

Error SocketImpl::Send(const Message& message, int timeout) {
  lock_guard<mutex> state_lock(state_mutex_);
  if (state_ == Socket::DISCONNECTING || state_ == Socket::DISCONNECTED) {
    return Error(LNR_ENOTCONN);
  }
  try {
    Message* copy_message;
    switch(message.type) {
    case linear::REQUEST:
      {
        Request* copy_request = new Request(dynamic_cast<const Request&>(message));
        copy_request->timeout = timeout;
        copy_message = copy_request;
      }
      break;
    case linear::RESPONSE:
      copy_message = new Response(dynamic_cast<const Response&>(message));
      break;
    case linear::NOTIFY:
      copy_message = new Notify(dynamic_cast<const Notify&>(message));
      break;
    default:
      LINEAR_LOG(LOG_ERR, "invalid type of message: %d", message.type);
      throw std::bad_typeid();
    }
    if (state_ == Socket::CONNECTING) {
      pending_messages_.push_back(copy_message);
      return Error(LNR_OK);
    }
    Error err = _Send(copy_message);
    if (err != Error(LNR_OK)) {
      delete copy_message;
    }
    return err;
  } catch(const std::bad_typeid&) {
    return Error(LNR_EINVAL);
  } catch(...) {
    return Error(LNR_ENOMEM);
  }
}

Error SocketImpl::KeepAlive(unsigned int interval, unsigned int retry, Socket::KeepAliveType type) {
  lock_guard<mutex> state_lock(state_mutex_);
  if (state_ != Socket::CONNECTING && state_ != Socket::CONNECTED) {
    return Error(LNR_ENOTCONN);
  }
  if (type == Socket::KEEPALIVE_WS && (type_ == Socket::WS || type_ == Socket::WSS)) {
    int ret = tv_ws_keepalive(stream_, 1, interval, retry);
    return Error(ret);
  }
  int ret = tv_keepalive(stream_, 1, interval, interval, retry);
  if (ret) {
    return Error(ret);
  }

#if defined(TCP_USER_TIMEOUT)
  int option = interval * 1000 * retry; // sec * retry
  ret = tv_setsockopt(stream_, IPPROTO_TCP, TCP_USER_TIMEOUT, (void*)&option, sizeof(option));
#else
  LINEAR_LOG(LOG_DEBUG, "TCP_USER_TIMEOUT is not supported on your system");
#endif

  return Error(ret);
}

Error SocketImpl::BindToDevice(const std::string& ifname) {
  bind_ifname_ = ifname;
  return Error(LNR_OK);
}

Error SocketImpl::SetSockOpt(int level, int optname, const void* optval, size_t optlen) {
  lock_guard<mutex> state_lock(state_mutex_);
  if (state_ != Socket::CONNECTING && state_ != Socket::CONNECTED) {
    return Error(LNR_ENOTCONN);
  }
  int ret = tv_setsockopt(stream_, level, optname, optval, optlen);
  if (ret != 0) {
    LINEAR_LOG(LOG_WARN, "fail to setsockopt(id = %d): %s\n",
               id_, tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
    return Error(LNR_EINVAL);
  }
  return Error(LNR_OK);
}

Error SocketImpl::StartRead(EventLoopImpl::SocketEvent* ev) {
  ev_ = ev;
  stream_->data = ev;
  int ret = tv_read_start(stream_, EventLoopImpl::OnRead);
  if (ret != 0) {
    assert(false); // never reach now
    tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoopImpl::OnClose);
    return Error(ret);
  }
  LINEAR_LOG(LOG_DEBUG, "connected(id = %d): %s:%d <-- %s --> %s:%d",
             id_,
             self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
             peer_.addr.c_str(), peer_.port);
  return Error(LNR_OK);
}

void SocketImpl::OnConnect(const shared_ptr<SocketImpl>& socket, tv_stream_t* stream, int status) {
  unique_lock<mutex> state_lock(state_mutex_);
  connect_timer_.Stop();
  if (state_ == Socket::CONNECTED) {
    return;
  }
  if (state_ != Socket::CONNECTING) {
    LINEAR_LOG(LOG_DEBUG, "connect(id = %d) is cancelled: x-- %s --> %s:%d",
               id_,
               GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
    return;
  }
  if (status) {
    state_ = Socket::DISCONNECTING;
    if (status == TV_EWS) {
      last_error_ = Error(LNR_EWS); // TODO: detail code
    } else {
      last_error_ = Error(status);
    }

#ifdef WITH_SSL
    if (status == TV_ESSL) {
      last_error_ = Error(LNR_ESSL, stream->ssl_err);
    } else if (status == TV_EX509) {
      last_error_ = Error(LNR_EX509, stream->ssl_err);
    }
#endif

    LINEAR_LOG(LOG_DEBUG, "fail to connect(id = %d), %s: --- %s --x %s:%d",
               id_,
               last_error_.Message().c_str(),
               GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
    state_lock.unlock();
    tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoopImpl::OnClose);
    return;
  }
  struct sockaddr_storage ss;
  int len = sizeof(struct sockaddr_storage);
  int ret = tv_getsockname(stream_, reinterpret_cast<struct sockaddr*>(&ss), &len);
  if (ret == 0) {
    self_ = Addrinfo(reinterpret_cast<struct sockaddr*>(&ss));
  }
  // OK.starts to read
  last_error_ = StartRead(ev_);
  if (last_error_ != Error(LNR_OK)) {
    LINEAR_LOG(LOG_DEBUG, "fail to connect(id = %d), %s: %s:%d --- %s --x %s:%d",
               id_,
               last_error_.Message().c_str(),
               self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
               peer_.addr.c_str(), peer_.port);
    return;
  }
  state_lock.unlock();
  // call OnConnect
  if (shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock()) {
    observer->Lock();
    HandlerDelegate* delegate = observer->GetSubject();
    if (delegate) {
      delegate->OnConnect(socket);
    }
    observer->Unlock();
  }
  state_lock.lock();
  if (state_ == Socket::CONNECTING) {
    state_ = Socket::CONNECTED;
  }
  state_lock.unlock();
  _SendPendingMessages(socket);
}

void SocketImpl::OnHandshakeComplete(const shared_ptr<SocketImpl>& socket, tv_stream_t* stream, int status) {
  unique_lock<mutex> state_lock(state_mutex_);
  if (status) {
    state_lock.unlock();
    Disconnect(true);
    return;
  }
  handshaking_ = false;
  state_ = Socket::CONNECTED;
  state_lock.unlock();
  _SendPendingMessages(socket);
}

void SocketImpl::OnDisconnect(const shared_ptr<SocketImpl>& socket) {
  unique_lock<mutex> state_lock(state_mutex_);
  connect_timer_.Stop();
  if (state_ == Socket::DISCONNECTED) {
    return;
  }
  LINEAR_LOG(LOG_DEBUG, "disconnected(id = %d): %s:%d x-- %s --x %s:%d",
             id_,
             self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
             peer_.addr.c_str(), peer_.port);
  state_ = Socket::DISCONNECTED;
  state_lock.unlock();
  shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock();
  if (observer) {
    observer->Lock();
    HandlerDelegate* delegate = observer->GetSubject();
    if (delegate) {
      delegate->Release(socket);
    }
    observer->Unlock();
  }
  // retry to connect for WebSocket DigestAuth
  if (connectable_) {
    if (type_ == Socket::WS) {
      if (last_error_ == Error(LNR_EWS)) {
        if (dynamic_cast<WSSocketImpl*>(this)->CheckRetryAuth()) {
          try {
            EventLoopImpl::SocketEvent* ev = new EventLoopImpl::SocketEvent(socket);
            Connect(connect_timeout_, ev);
          } catch(...) {
            LINEAR_LOG(LOG_ERR, "no memory");
            WSResponseContext ctx;
            ctx.code = LNR_WS_INTERNAL_SERVER_ERROR;
            try {
              dynamic_cast<WSSocketImpl*>(this)->SetWSResponseContext(ctx);
            } catch(...) {}
          }
          return;
        }
      } else {
        WSResponseContext ctx;
        ctx.code = LNR_WS_SERVICE_UNAVAILABLE;
        try {
          dynamic_cast<WSSocketImpl*>(this)->SetWSResponseContext(ctx);
        } catch(...) {}
      }

#ifdef WITH_SSL
    } else if (type_ == Socket::WSS) {
      if (last_error_ == Error(LNR_EWS)) {
        if (dynamic_cast<WSSSocketImpl*>(this)->CheckRetryAuth()) {
          try {
            EventLoopImpl::SocketEvent* ev = new EventLoopImpl::SocketEvent(socket);
            Connect(connect_timeout_, ev);
          } catch(...) {
            LINEAR_LOG(LOG_ERR, "no memory");
            WSResponseContext ctx;
            ctx.code = LNR_WS_INTERNAL_SERVER_ERROR;
            try {
              dynamic_cast<WSSSocketImpl*>(this)->SetWSResponseContext(ctx);
            } catch(...) {}
          }
          return;
        }
      } else {
        WSResponseContext ctx;
        ctx.code = LNR_WS_SERVICE_UNAVAILABLE;
        try {
          dynamic_cast<WSSSocketImpl*>(this)->SetWSResponseContext(ctx);
        } catch(...) {}
      }
#endif

    }
  }
  _DiscardMessages(socket);
  if (observer && !handshaking_) {
    observer->Lock();
    HandlerDelegate* delegate = observer->GetSubject();
    if (delegate) {
      delegate->OnDisconnect(socket, last_error_);
    }
    observer->Unlock();
  }
  self_ = Addrinfo(); // reset self info
  return;
}

class _Response : public Message {
 public:
  _Response() : Message(linear::RESPONSE), msgid(0) {}
  ~_Response() {}

 public:
  uint32_t msgid;
  linear::type::any result;
  linear::type::any error;
  MSGPACK_DEFINE(type, msgid, error, result);
};

void SocketImpl::OnRead(const shared_ptr<SocketImpl>& socket, const tv_buf_t* buffer, ssize_t nread) {
  unique_lock<mutex> state_lock(state_mutex_);
  if (state_ != Socket::CONNECTING && state_ != Socket::CONNECTED) {
    if (nread > 0) {
      free(buffer->base);
    }
    return;
  }
  state_lock.unlock();

  Error e(nread);
#ifdef WITH_SSL
  if (nread == TV_ESSL) {
    e = Error(LNR_ESSL, stream_->ssl_err);
  }
#endif

  assert(nread != 0);
  if (nread <= 0) {
    LINEAR_LOG(LOG_DEBUG, "%s(id = %d): %s:%d --- %s --x %s:%d",
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), nread), id_,
               self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
    // error or EOF
    Disconnect(handshaking_);
    last_error_ = e;
    return;
  }
  // nread > 0
  unpacker_.reserve_buffer(nread);
  memcpy(unpacker_.buffer(), buffer->base, nread);
  free(buffer->base);
  unpacker_.buffer_consumed(nread);
  HandlerDelegate* delegate = NULL;
  shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock();
  if (observer) {
    observer->Lock();
    delegate = observer->GetSubject();
  }
  try {
    msgpack::unpacked result;
    while (unpacker_.next(&result)) {
      msgpack::object obj = result.get();
      std::auto_ptr<msgpack::zone> zone = result.zone();
      Message message = obj.as<Message>();
      switch(message.type) {
      case REQUEST:
        {
          Request request = obj.as<Request>();
          LINEAR_LOG(LOG_DEBUG, "recv request(id = %d): msgid = %u, method = \"%s\", params = %s, %s:%d <-- %s --- %s:%d",
                     id_, request.msgid,
                     request.method.c_str(), LINEAR_LOG_PRINTABLE_STRING(request.params).c_str(),
                     self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
          if (delegate) {
            delegate->OnMessage(socket, request);
          }
        }
        break;
      case RESPONSE:
        {
          _Response _response = obj.as<_Response>();
          LINEAR_LOG(LOG_DEBUG, "recv response(id = %d): msgid = %u, result = %s, error = %s, %s:%d <-- %s --- %s:%d",
                     id_, _response.msgid,
                     LINEAR_LOG_PRINTABLE_STRING(_response.result).c_str(),
                     LINEAR_LOG_PRINTABLE_STRING(_response.error).c_str(),
                     self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
          unique_lock<mutex> request_timer_lock(request_timer_mutex_);
          for (std::vector<SocketImpl::RequestTimer*>::iterator it = request_timers_.begin();
               it != request_timers_.end(); it++) {
            const Request& request = (*it)->request;
            if (request.msgid == _response.msgid) {
              Response response(_response.msgid, _response.result, _response.error, request);
              delete *it;
              request_timers_.erase(it);
              request_timer_lock.unlock();
              if (delegate) {
                delegate->OnMessage(socket, response);
              }
              break;
            }
          }
        }
        break;
      case NOTIFY:
        {
          Notify notify = obj.as<Notify>();
          LINEAR_LOG(LOG_DEBUG, "recv notify(id = %d): method = \"%s\", params = %s, %s:%d <-- %s --- %s:%d",
                     id_,
                     notify.method.c_str(), LINEAR_LOG_PRINTABLE_STRING(notify.params).c_str(),
                     self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
          if (delegate) {
            delegate->OnMessage(socket, notify);
          }
          break;
        }
        break;
      default:
        throw std::bad_cast();
      }
    }
    if (unpacker_.message_size() > max_buffer_size_) {
      throw std::runtime_error("");
    }
  } catch (const std::bad_cast&) {
    LINEAR_LOG(LOG_WARN, "recv invalid message(id = %d): %s:%d <-- %s -- %s:%d",
               id_,
               self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
               peer_.addr.c_str(), peer_.port);
    Disconnect();
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "recv malformed or big message(id = %d): %s:%d <-- %s -- %s:%d",
               id_,
               self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
               peer_.addr.c_str(), peer_.port);
    Disconnect();
  }
  if (observer) {
    observer->Unlock();
  }
}

void SocketImpl::OnWrite(const shared_ptr<SocketImpl>& socket, const Message* message, int status) {
  assert(message != NULL);
  if (status) {
    LINEAR_LOG(LOG_ERR, "fail to send message(id = %d): %s",
               id_,
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), status));
    HandlerDelegate* delegate = NULL;
    shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock();
    if (observer) {
      observer->Lock();
      delegate = observer->GetSubject();
    }
    if (delegate) {
      switch(message->type) {
      case REQUEST:
        delegate->OnError(socket, *(dynamic_cast<const Request*>(message)), Error(status));
        break;
      case RESPONSE:
        delegate->OnError(socket, *(dynamic_cast<const Response*>(message)), Error(status));
        break;
      case NOTIFY:
        delegate->OnError(socket, *(dynamic_cast<const Notify*>(message)), Error(status));
        break;
      default:
        LINEAR_LOG(LOG_ERR, "BUG: invalid type of message");
        assert(false);
      }
    }
    if (observer) {
      observer->Unlock();
    }
  }
}

void SocketImpl::OnConnectTimeout(const shared_ptr<SocketImpl>& socket) {
  OnConnect(socket, stream_, TV_ETIMEDOUT);
}

void SocketImpl::OnRequestTimeout(const shared_ptr<SocketImpl>& socket, const Request& request) {
  unique_lock<mutex> request_timer_lock(request_timer_mutex_);
  for (std::vector<SocketImpl::RequestTimer*>::iterator it = request_timers_.begin();
       it != request_timers_.end(); it++) {
    const Request& ref = (*it)->request;
    if (ref.msgid == request.msgid) {
      LINEAR_LOG(LOG_INFO, "occur request timeout(id = %d): msgid = %d",
                 id_, request.msgid);
      request_timers_.erase(it);
      break;
    }
  }
  request_timer_lock.unlock();
  if (shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock()) {
    observer->Lock();
    HandlerDelegate* delegate = observer->GetSubject();
    if (delegate) {
      delegate->OnError(socket, request, Error(LNR_ETIMEDOUT));
    }
    observer->Unlock();
  }
}

Error SocketImpl::_Send(Message* message) {
  assert(message != NULL);
  msgpack::sbuffer sbuf;
  switch(message->type) {
  case REQUEST:
    {
      const Request* request = dynamic_cast<const Request*>(message);
      LINEAR_LOG(LOG_DEBUG, "send request(id = %d): msgid = %u, method = \"%s\", params = %s, %s:%d --- %s --> %s:%d",
                 id_,
                 request->msgid, request->method.c_str(), LINEAR_LOG_PRINTABLE_STRING(request->params).c_str(),
                 self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
      msgpack::pack(sbuf, *request);
      break;
    }
  case RESPONSE:
    {
      const Response* response = dynamic_cast<const Response*>(message);
      LINEAR_LOG(LOG_DEBUG, "send response(id = %d): msgid = %u, result = %s, error = %s, %s:%d --- %s --> %s:%d",
                 id_,
                 response->msgid,
                 LINEAR_LOG_PRINTABLE_STRING(response->result).c_str(),
                 LINEAR_LOG_PRINTABLE_STRING(response->error).c_str(),
                 self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
      msgpack::pack(sbuf, *response);
      break;
    }
  case NOTIFY:
    {
      const Notify* notify = dynamic_cast<const Notify*>(message);
      LINEAR_LOG(LOG_DEBUG, "send notify(id = %d): method = \"%s\", params = %s, %s:%d --- %s --> %s:%d",
                 id_,
                 notify->method.c_str(), LINEAR_LOG_PRINTABLE_STRING(notify->params).c_str(),
                 self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
      msgpack::pack(sbuf, *notify);
      break;
    }
  default:
    LINEAR_LOG(LOG_ERR, "invalid type of message: %d", message->type);
    return Error(LNR_EINVAL);
  }
  char* copy_data = static_cast<char*>(malloc(sbuf.size()));
  if (copy_data == NULL) {
    Error err(LNR_ENOMEM);
    LINEAR_LOG(LOG_ERR, "fail to send message(id = %d): %s",
               id_, err.Message().c_str());
    return err;
  }
  memcpy(copy_data, sbuf.data(), sbuf.size());
  tv_buf_t buffer = static_cast<tv_buf_t>(uv_buf_init(copy_data, sbuf.size()));
  tv_write_t* w = static_cast<tv_write_t*>(malloc(sizeof(tv_write_t)));
  if (w == NULL) {
    free(copy_data);
    Error err(LNR_ENOMEM);
    LINEAR_LOG(LOG_ERR, "fail to send message(id = %d): %s",
               id_, err.Message().c_str());
    return err;
  }
  w->data = message;
  if (message->type == REQUEST) {
    const Request* request = dynamic_cast<const Request*>(message);
    try {
      RequestTimer* request_timer = new RequestTimer(*request, ev_->socket);
      unique_lock<mutex> request_timer_lock(request_timer_mutex_);
      request_timers_.push_back(request_timer);
      request_timer_lock.unlock();
      request_timer->Start();
    } catch(...) {
      free(w);
      free(copy_data);
      Error err(LNR_ENOMEM);
      LINEAR_LOG(LOG_ERR, "fail to send message(id = %d): %s",
                 id_, err.Message().c_str());
      return err;
    }
  }
  int ret = tv_write(w, stream_, buffer, EventLoopImpl::OnWrite);
  if (ret) { // EINVAL or ENOMEM
    free(w);
    free(copy_data);
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to send message(id = %d): %s",
               id_, err.Message().c_str());
    return err;
  }
  return Error(LNR_OK);
}

void SocketImpl::_SendPendingMessages(const shared_ptr<SocketImpl>& socket) {
  unique_lock<mutex> state_lock(state_mutex_);
  // Send pending messages
  std::vector<Message*> fail_to_send;
  for (std::vector<Message*>::iterator it = pending_messages_.begin();
       it != pending_messages_.end(); it++) {
    if (state_ != Socket::CONNECTED) {
      fail_to_send.push_back(*it);
    } else {
      Error err = _Send(*it);
      if (err != Error(LNR_OK)) {
        fail_to_send.push_back(*it);
      }
    }
  }
  std::vector<Message*>().swap(pending_messages_);
  state_lock.unlock();
  // call OnError when fail to send pending messages
  HandlerDelegate* delegate = NULL;
  shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock();
  if (observer) {
    observer->Lock();
    delegate = observer->GetSubject();
  }
  Error pending_err = Error(LNR_ECANCELED);
  for (std::vector<Message*>::iterator it = fail_to_send.begin();
       it != fail_to_send.end(); it++) {
    Message* message = *it;
    if (delegate) {
      switch(message->type) {
      case REQUEST:
        delegate->OnError(socket, *(dynamic_cast<Request*>(message)), pending_err);
        break;
      case RESPONSE:
        delegate->OnError(socket, *(dynamic_cast<Response*>(message)), pending_err);
        break;
      case NOTIFY:
        delegate->OnError(socket, *(dynamic_cast<Notify*>(message)), pending_err);
        break;
      default:
        LINEAR_LOG(LOG_ERR, "BUG: invalid type of message");
        assert(false);
      }
    }
    delete message;
  }
  if (observer) {
    observer->Unlock();
  }
}

void SocketImpl::_DiscardMessages(const shared_ptr<SocketImpl>& socket) {
  HandlerDelegate* delegate = NULL;
  shared_ptr<Observer<HandlerDelegate> > observer = observer_.lock();
  if (observer) {
    observer->Lock();
    delegate = observer->GetSubject();
  }
  Error err = Error(LNR_ECANCELED);
  std::vector<Message*> fail_to_send;
  fail_to_send.swap(pending_messages_);
  for (std::vector<Message*>::iterator it = fail_to_send.begin();
       it != fail_to_send.end(); it++) {
    Message* message = *it;
    if (delegate) {
      switch(message->type) {
      case REQUEST:
        delegate->OnError(socket, *(dynamic_cast<Request*>(message)), err);
        break;
      case RESPONSE:
        delegate->OnError(socket, *(dynamic_cast<Response*>(message)), err);
        break;
      case NOTIFY:
        delegate->OnError(socket, *(dynamic_cast<Notify*>(message)), err);
        break;
      default:
        LINEAR_LOG(LOG_ERR, "BUG: invalid type of message");
        assert(false);
      }
    }
    delete message;
  }

  std::vector<RequestTimer*> cancelled_requests;
  unique_lock<mutex> request_timer_lock(request_timer_mutex_);
  cancelled_requests.swap(request_timers_);
  request_timer_lock.unlock();
  for (std::vector<RequestTimer*>::iterator it = cancelled_requests.begin();
       it != cancelled_requests.end(); it++) {
    if (delegate) {
      delegate->OnError(socket, (*it)->request, err);
    }
    delete *it;
  }
  if (observer) {
    observer->Unlock();
  }
}

}  // namespace linear
