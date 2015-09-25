#include <sstream>

#include "linear/log.h"
#include "linear/group.h"
#include "linear/ws_socket.h"

#include "socket_impl.h"
#include "ws_socket_impl.h"

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
SocketImpl::SocketImpl(const std::string& host, int port, const HandlerDelegate& delegate, Socket::Type type)
  : stream_(NULL), data_(NULL), peer_(Addrinfo(host, port)), bind_ifname_(), type_(type), id_(Id()),
    connectable_(true), handshaking_(false), last_error_(LNR_OK), state_(Socket::DISCONNECTED), observer_(delegate.GetObserver()),
    max_buffer_size_(Socket::DEFAULT_MAX_BUFFER_SIZE) {
}

// Server Socket
SocketImpl::SocketImpl(tv_stream_t* stream, const HandlerDelegate& delegate, Socket::Type type)
  : stream_(stream), data_(NULL), bind_ifname_(), type_(type), id_(Id()),
    connectable_(false), last_error_(LNR_OK), observer_(delegate.GetObserver()),
    max_buffer_size_(Socket::DEFAULT_MAX_BUFFER_SIZE) {
  if (type == Socket::WS) {
    handshaking_ = true;
    state_ = Socket::CONNECTING;
    reinterpret_cast<tv_ws_t*>(stream_)->handshake_complete_cb = EventLoop::OnAcceptComplete;

#ifdef WITH_SSL
  } else if (type == Socket::WSS) {
    handshaking_ = true;
    state_ = Socket::CONNECTING;
    reinterpret_cast<tv_wss_t*>(stream_)->handshake_complete_cb = EventLoop::OnAcceptComplete;
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
  } else {
    LINEAR_LOG(LOG_ERR, "BUG: fail to get sockinfo: %s",
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
  }
  ret = tv_getpeername(stream_, reinterpret_cast<struct sockaddr*>(&ss), &len);
  if (ret == 0) {
    peer_ = Addrinfo(reinterpret_cast<struct sockaddr*>(&ss));
  } else {
    LINEAR_LOG(LOG_WARN, "fail to get sockinfo (may disconnected by peer): %s",
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
  }
  // overwrite peer_
  if (type == Socket::WS) {
    tv_ws_t* ws_handle = reinterpret_cast<tv_ws_t*>(stream_);
    const buffer* val = buffer_kvs_case_find(&ws_handle->handshake.request.headers, CONST_STRING("X-Forwarded-For"));
    if (val) {
      peer_.addr = std::string(val->ptr);
      // store first element
      std::replace(peer_.addr.begin(), peer_.addr.end(), ',', ' ');
      std::stringstream ss(peer_.addr);
      ss >> peer_.addr;
      if (peer_.addr.find(":", 0) != std::string::npos) {
        std::replace(peer_.addr.begin(), peer_.addr.end(), ':', ' ');
        ss.str("");
        ss.clear(std::stringstream::goodbit);
        ss << peer_.addr;
        ss >> peer_.addr >> peer_.port;
      } else {
        // overwrite port when X-Forwarded-For exists
        val = buffer_kvs_case_find(&ws_handle->handshake.request.headers, CONST_STRING("X-Forwarded-Port"));
        if (val) {
          peer_.port = atoi(val->ptr);
        } else {
          peer_.port = -1;
        }
      }
    }

#ifdef WITH_SSL
  } else if (type == Socket::WSS) {
    tv_wss_t* ws_handle = reinterpret_cast<tv_wss_t*>(stream_);
    const buffer* val = buffer_kvs_case_find(&ws_handle->handshake.request.headers, CONST_STRING("X-Forwarded-For"));
    if (val) {
      peer_.addr = std::string(val->ptr);
      // store first element
      std::replace(peer_.addr.begin(), peer_.addr.end(), ',', ' ');
      std::stringstream ss(peer_.addr);
      ss >> peer_.addr;
      if (peer_.addr.find(":", 0) != std::string::npos) {
        std::replace(peer_.addr.begin(), peer_.addr.end(), ':', ' ');
        ss.str("");
        ss.clear(std::stringstream::goodbit);
        ss << peer_.addr;
        ss >> peer_.addr >> peer_.port;
      } else {
        // overwrite port when X-Forwarded-For exists
        val = buffer_kvs_case_find(&ws_handle->handshake.request.headers, CONST_STRING("X-Forwarded-Port"));
        if (val) {
          peer_.port = atoi(val->ptr);
        } else {
          peer_.port = -1;
        }
      }
    }
#endif

  }

  LINEAR_LOG(LOG_DEBUG, "incoming peer: %s:%d", peer_.addr.c_str(), peer_.port);
  try {
    data_ = new EventLoop::SocketEventData();
  } catch(...) {
    throw;
  }
  data_->Register(this);
  stream_->data = data_;
  ret = tv_read_start(stream_, EventLoop::OnRead);
  if (ret != 0) {
    data_->Unregister();
    LINEAR_LOG(LOG_ERR, "fail to read_start: %s",
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
    tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoop::OnClose);
    stream_ = NULL; // unref stream
    throw ret;
  }
  LINEAR_LOG(LOG_DEBUG, "connected: %s:%d <-- %s --> %s:%d",
             self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
}

SocketImpl::~SocketImpl() {
  unique_lock<mutex> state_lock(state_mutex_);
  state_ = Socket::DISCONNECTED;
  state_lock.unlock();
  EventLoop& loop = const_cast<EventLoop&>(EventLoop::GetDefault());
  if (loop.GetHandle() != NULL) {
    loop.Lock();
    if (data_) {
      data_->Lock();
      data_->Unregister();
      data_->Unlock();
    }
    if (stream_) {
      LINEAR_LOG(LOG_WARN, "Connected socket(id = %d) has existed yet. automatically correct this.", id_);
      tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoop::OnClose);
      stream_ = NULL; // unref stream
    }
    loop.Unlock();
  }
  _DiscardMessages(Socket());
}

void SocketImpl::SetMaxBufferSize(size_t max_limit) {
  max_buffer_size_ = max_limit;
}

Error SocketImpl::Connect(unsigned int timeout, const Socket& socket) {
  lock_guard<mutex> state_lock(state_mutex_);
  LINEAR_LOG(LOG_DEBUG, "try to connect to %s:%d,%s", peer_.addr.c_str(), peer_.port, GetTypeString(type_).c_str());
  if (!connectable_) {
    LINEAR_LOG(LOG_WARN, "this socket is not connectable");
    return Error(LNR_EINVAL);
  }
  if (state_ == Socket::CONNECTING || state_ == Socket::CONNECTED) {
    LINEAR_LOG(LOG_INFO, "this socket is %s",
               (state_ == Socket::CONNECTING) ? "connecting now" : "already connected");
    return Error(LNR_EALREADY);
  } else if (state_ == Socket::DISCONNECTING) {
    LINEAR_LOG(LOG_WARN, "this socket is disconnecting now.plz call later.");
    return Error(LNR_EBUSY);
  }
  bool is_locked = observer_->TryLock();
  if (!is_locked) {
    LINEAR_LOG(LOG_DEBUG, "fail to delegate lock.(may be already locked)");
  }
  HandlerDelegate* delegate = observer_->GetSubject();
  if (delegate) {
    delegate->Retain(socket);
  }
  if (is_locked) {
    observer_->Unlock();
  }
  Error err = Connect();
  if (err.Code() == LNR_OK) {
    state_ = Socket::CONNECTING;
    if (timeout > 0) {
      connect_timer_.Start(EventLoop::OnConnectTimeout, timeout, this);
    }
  } else {
    LINEAR_LOG(LOG_ERR, "fail to connect: %s", err.Message().c_str());
    is_locked = observer_->TryLock();
    if (is_locked) {
      LINEAR_LOG(LOG_DEBUG, "fail to delegate lock.(may be already locked)");
    }
    HandlerDelegate* delegate = observer_->GetSubject();
    if (delegate) {
      delegate->Release(socket.GetId());
    }
    if (is_locked) {
      observer_->Unlock();
    }
  }
  return err;
}

Error SocketImpl::Disconnect(bool handshaking) {
  lock_guard<mutex> state_lock(state_mutex_);
  handshaking_ = handshaking;
  if (state_ == Socket::DISCONNECTING || state_ == Socket::DISCONNECTED) {
    LINEAR_LOG(LOG_INFO, "this socket is %s",
               (state_ == Socket::DISCONNECTING) ? "disconnecting now" : "already disconnected");
    return Error(LNR_EALREADY);
  }
  connect_timer_.Stop();
  state_ = Socket::DISCONNECTING;
  last_error_ = Error(LNR_OK);
  tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoop::OnClose);
  stream_ = NULL; // unref stream
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
      LINEAR_LOG(LOG_ERR, "message type is invalid: %d", message.type);
      throw std::bad_typeid();
    }
    if (state_ == Socket::CONNECTING) {
      pending_messages_.push_back(copy_message);
      return Error(LNR_OK);
    }
    Error err = _Send(copy_message);
    if (err.Code() != LNR_OK) {
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
  int ret = tv_setsockopt(stream_, level, optname, optval, optlen);
  if (ret != 0) {
    LINEAR_LOG(LOG_WARN, "fail to setsockopt(%s)\n",
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
    return Error(LNR_EINVAL);
  }
  return Error(LNR_OK);
}

void SocketImpl::OnConnect(tv_stream_t* stream, int status) {
  unique_lock<mutex> state_lock(state_mutex_);
  connect_timer_.Stop();
  if (state_ != Socket::CONNECTING) {
    state_lock.unlock();
    LINEAR_LOG(LOG_DEBUG, "connect to %s:%d,%s is cancelled",
               peer_.addr.c_str(), peer_.port, GetTypeString(type_).c_str());
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

    LINEAR_LOG(LOG_ERR, "fail to connect to %s:%d,%s: %s",
               peer_.addr.c_str(), peer_.port, GetTypeString(type_).c_str(),
               last_error_.Message().c_str());

    state_lock.unlock();
    tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoop::OnClose);
    stream_ = NULL; // unref stream
    return;
  }

  // OK.starts to read
  int ret = tv_read_start(stream_, EventLoop::OnRead);
  if (ret != 0) { // never reach now
    assert(false);
    LINEAR_LOG(LOG_ERR, "fail to connect to %s:%d,%s: %s",
               peer_.addr.c_str(), peer_.port, GetTypeString(type_).c_str(),
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
    state_ = Socket::DISCONNECTING;
    last_error_ = Error(ret);
    state_lock.unlock();
    tv_close(reinterpret_cast<tv_handle_t*>(stream_), EventLoop::OnClose);
    stream_ = NULL; // unref stream
    return;
  }
  struct sockaddr_storage ss;
  int len = sizeof(struct sockaddr_storage);
  ret = tv_getsockname(stream_, reinterpret_cast<struct sockaddr*>(&ss), &len);
  if (ret == 0) {
    self_ = Addrinfo(reinterpret_cast<struct sockaddr*>(&ss));
  } else {
    LINEAR_LOG(LOG_WARN, "fail to get sockinfo: %s",
               tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), ret));
  }
  LINEAR_LOG(LOG_DEBUG, "connected: %s:%d <-- %s --> %s:%d",
             self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
             peer_.addr.c_str(), peer_.port);
  state_lock.unlock();

  // call OnConnect
  observer_->Lock();
  HandlerDelegate* delegate = observer_->GetSubject();
  if (delegate) {
    delegate->OnConnect(id_);
  }
  observer_->Unlock();
  state_lock.lock();
  if (state_ == Socket::CONNECTING) {
    state_ = Socket::CONNECTED;
  }
  state_lock.unlock();
  _SendPendingMessages();
}

void SocketImpl::OnHandshakeComplete(tv_stream_t* stream, int status) {
  unique_lock<mutex> state_lock(state_mutex_);
  if (status) {
    state_lock.unlock();
    Disconnect(true);
    return;
  }
  handshaking_ = false;
  state_ = Socket::CONNECTED;
  state_lock.unlock();
  _SendPendingMessages();
}

Socket SocketImpl::OnDisconnect() {
  unique_lock<mutex> state_lock(state_mutex_);
  if (state_ == Socket::DISCONNECTED) {
    return Socket();
  }
  LINEAR_LOG(LOG_DEBUG, "disconnected: %s:%d <-- %s --> %s:%d",
             self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
             peer_.addr.c_str(), peer_.port);
  state_ = Socket::DISCONNECTED;
  self_ = Addrinfo(); // reset self info
  observer_->Lock();
  linear::Socket socket; // need to copy socket from SocketPool
  HandlerDelegate* delegate = observer_->GetSubject();
  if (delegate) {
    socket = delegate->Get(id_);
    delegate->Release(id_);
  }
  observer_->Unlock();
  state_lock.unlock();

  if (connectable_) {
    if (type_ == Socket::WS) {
      if (last_error_ == Error(LNR_EWS)) {
        if (dynamic_cast<WSSocketImpl*>(this)->CheckRetryAuth()) {
          socket.Connect();
          return socket;
        }
      } else {
        WSResponseContext ctx;
        ctx.code = LNR_WS_SERVICE_UNAVAILABLE;
        try {
          socket.as<WSSocket>().SetWSResponseContext(ctx);
        } catch(...) {}
      }

#ifdef WITH_SSL
    } else if (type_ == Socket::WSS) {
      if (last_error_ == Error(LNR_EWS)) {
        if (dynamic_cast<WSSSocketImpl*>(this)->CheckRetryAuth()) {
          socket.Connect();
          return socket;
        }
      } else {
        WSResponseContext ctx;
        ctx.code = LNR_WS_SERVICE_UNAVAILABLE;
        try {
          socket.as<WSSSocket>().SetWSResponseContext(ctx);
        } catch(...) {}
      }
#endif

    }
  }

  _DiscardMessages(socket);
  observer_->Lock();
  delegate = observer_->GetSubject();
  if (delegate && !handshaking_) {
    delegate->OnDisconnect(socket, last_error_);
  }
  observer_->Unlock();
  return socket;
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

void SocketImpl::OnRead(const tv_buf_t* buffer, ssize_t nread) {
  unique_lock<mutex> state_lock(state_mutex_);
  if (state_ != Socket::CONNECTING && state_ != Socket::CONNECTED) {
    if (nread > 0) {
      free(buffer->base);
    }
    return;
  }
  Error e(nread);

#ifdef WITH_SSL
  if (nread == TV_ESSL) {
    e = Error(LNR_ESSL, stream_->ssl_err);
  }
#endif

  state_lock.unlock();
  try {
    assert(nread != 0);
    if (nread < 0) {
      LINEAR_LOG(LOG_DEBUG, "%s: %s:%d <-- %s --- %s:%d",
                 tv_strerror(reinterpret_cast<tv_handle_t*>(stream_), nread),
                 self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
      // error or EOF
      Disconnect(handshaking_);
      last_error_ = e;
    } else if (nread > 0) {
      unpacker_.reserve_buffer(nread);
      memcpy(unpacker_.buffer(), buffer->base, nread);
      free(buffer->base);
      unpacker_.buffer_consumed(nread);
      msgpack::unpacked result;
      while (unpacker_.next(&result)) {
        msgpack::object obj = result.get();
        std::auto_ptr<msgpack::zone> zone = result.zone();
        // ReThink: use const ref (NVRO issue)
        Message message = obj.as<Message>();
        switch(message.type) {
        case REQUEST:
          {
            Request request = obj.as<Request>();
            LINEAR_LOG(LOG_DEBUG, "recv request: msgid = %u, method = \"%s\", params = %s, %s:%d <-- %s --- %s:%d",
                       request.msgid, request.method.c_str(), LINEAR_LOG_PRINTABLE_STRING(request.params).c_str(),
                       self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
            observer_->Lock();
            HandlerDelegate* delegate = observer_->GetSubject();
            if (delegate) {
              delegate->OnMessage(id_, request);
            }
            observer_->Unlock();
          }
          break;
        case RESPONSE:
          {
            _Response _response = obj.as<_Response>();
            LINEAR_LOG(LOG_DEBUG, "recv response: msgid = %u, result = %s, error = %s, %s:%d <-- %s --- %s:%d",
                       _response.msgid,
                       LINEAR_LOG_PRINTABLE_STRING(_response.result).c_str(),
                       LINEAR_LOG_PRINTABLE_STRING(_response.error).c_str(),
                       self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
            unique_lock<mutex> request_timer_lock(request_timer_mutex_);
            for (std::list<SocketImpl::RequestTimer*>::iterator it = request_timers_.begin();
                 it != request_timers_.end(); it++) {
              const Request& request = (*it)->GetRequest();
              if (request.msgid == _response.msgid) {
                Response response(_response.msgid, _response.result, _response.error, request);
                delete *it;
                request_timers_.erase(it);
                request_timer_lock.unlock();
                observer_->Lock();
                HandlerDelegate* delegate = observer_->GetSubject();
                if (delegate) {
                  delegate->OnMessage(id_, response);
                }
                observer_->Unlock();
                break;
              }
            }
          }
          break;
        case NOTIFY:
          {
            Notify notify = obj.as<Notify>();
            LINEAR_LOG(LOG_DEBUG, "recv notify: method = \"%s\", params = %s, %s:%d <-- %s --- %s:%d",
                       notify.method.c_str(), LINEAR_LOG_PRINTABLE_STRING(notify.params).c_str(),
                       self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
            observer_->Lock();
            HandlerDelegate* delegate = observer_->GetSubject();
            if (delegate) {
              delegate->OnMessage(id_, notify);
            }
            observer_->Unlock();
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
    }
  } catch (const std::bad_cast&) {
    LINEAR_LOG(LOG_WARN, "recv invalid message from %s:%d <-- %s -- %s:%d",
               self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(),
               peer_.addr.c_str(), peer_.port);
    Disconnect();
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "recv malformed packet or message size is too big");
    Disconnect();
  }
}

void SocketImpl::OnWrite(const Message* message, int status) {
  assert(message != NULL);
  if (status) {
    observer_->Lock();
    HandlerDelegate* delegate = observer_->GetSubject();
    if (delegate) {
      Socket socket = delegate->Get(id_);
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
    observer_->Unlock();
  }
  delete message;
}

void SocketImpl::OnConnectTimeout() {
  OnConnect(stream_, TV_ETIMEDOUT);
}

void SocketImpl::OnRequestTimeout(const Request& request) {
  unique_lock<mutex> request_timer_lock(request_timer_mutex_);
  for (std::list<SocketImpl::RequestTimer*>::iterator it = request_timers_.begin();
       it != request_timers_.end(); it++) {
    const Request& ref = (*it)->GetRequest();
    if (ref.msgid == request.msgid) {
      LINEAR_LOG(LOG_INFO, "occur request timeout: msgid = %d", request.msgid);
      request_timers_.erase(it);
      break;
    }
  }
  request_timer_lock.unlock();
  observer_->Lock();
  HandlerDelegate* delegate = observer_->GetSubject();
  if (delegate) {
    Socket socket = delegate->Get(id_);
    delegate->OnError(socket, request, Error(LNR_ETIMEDOUT));
  }
  observer_->Unlock();
}

Error SocketImpl::_Send(Message* message) {
  assert(message != NULL);
  if (message == NULL) {
    return Error(LNR_EINVAL);
  }
  msgpack::sbuffer sbuf;
  switch(message->type) {
  case REQUEST:
    {
      const Request* request = dynamic_cast<const Request*>(message);
      assert(request != NULL);
      if (request == NULL) {
        return Error(LNR_EINVAL);
      }
      LINEAR_LOG(LOG_DEBUG, "send request: msgid = %u, method = \"%s\", params = %s, %s:%d --- %s --> %s:%d",
                 request->msgid, request->method.c_str(), LINEAR_LOG_PRINTABLE_STRING(request->params).c_str(),
                 self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
      msgpack::pack(sbuf, *request);
      break;
    }
  case RESPONSE:
    {
      const Response* response = dynamic_cast<const Response*>(message);
      assert(response != NULL);
      if (response == NULL) {
        return Error(LNR_EINVAL);
      }
      LINEAR_LOG(LOG_DEBUG, "send response: msgid = %u, result = %s, error = %s, %s:%d --- %s --> %s:%d",
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
      assert(notify != NULL);
      if (notify == NULL) {
        return Error(LNR_EINVAL);
      }
      LINEAR_LOG(LOG_DEBUG, "send notify: method = \"%s\", params = %s, %s:%d --- %s --> %s:%d",
                 notify->method.c_str(), LINEAR_LOG_PRINTABLE_STRING(notify->params).c_str(),
                 self_.addr.c_str(), self_.port, GetTypeString(type_).c_str(), peer_.addr.c_str(), peer_.port);
      msgpack::pack(sbuf, *notify);
      break;
    }
  default:
    LINEAR_LOG(LOG_ERR, "message type is invalid: %d", message->type);
    return Error(LNR_EINVAL);
  }
  char* copy_data = static_cast<char*>(malloc(sbuf.size()));
  if (copy_data == NULL) {
    Error err(LNR_ENOMEM);
    LINEAR_LOG(LOG_ERR, "fail to send: %s", err.Message().c_str());
    return err;
  }
  memcpy(copy_data, sbuf.data(), sbuf.size());
  tv_buf_t buffer = static_cast<tv_buf_t>(uv_buf_init(copy_data, sbuf.size()));
  tv_write_t* w = static_cast<tv_write_t*>(malloc(sizeof(tv_write_t)));
  if (w == NULL) {
    free(copy_data);
    Error err(LNR_ENOMEM);
    LINEAR_LOG(LOG_ERR, "fail to send: %s", err.Message().c_str());
    return err;
  }
  w->data = message;
  RequestTimer* request_timer = NULL;
  if (message->type == REQUEST) {
    const Request* request = dynamic_cast<const Request*>(message);
    assert(request != NULL);
    if (request == NULL) {
      free(w);
      free(copy_data);
      return Error(LNR_EINVAL);
    }
    try {
      request_timer = new RequestTimer(*request, this);
    } catch(...) {
      free(w);
      free(copy_data);
      return Error(LNR_ENOMEM);
    }
    unique_lock<mutex> request_timer_lock(request_timer_mutex_);
    request_timers_.push_back(request_timer);
    request_timer_lock.unlock();
    request_timer->Start();
  }
  int ret = tv_write(w, stream_, buffer, EventLoop::OnWrite);
  if (ret) { // EINVAL or ENOMEM
    if (message->type == REQUEST) {
      unique_lock<mutex> request_timer_lock(request_timer_mutex_);
      request_timers_.pop_back();
      request_timer_lock.unlock();
      if (request_timer != NULL) {
        delete request_timer;
      }
    }
    free(w);
    free(copy_data);
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to send: %s", err.Message().c_str());
    return err;
  }
  return Error(LNR_OK);
}

void SocketImpl::UnrefResources() {
  lock_guard<mutex> state_lock(state_mutex_);
  if (state_ == Socket::DISCONNECTED) {
    stream_ = NULL; // unref tv_stream
    data_ = NULL; // unref SocketEventData
  }
}

void SocketImpl::_SendPendingMessages() {
  unique_lock<mutex> state_lock(state_mutex_);
  // Send pending messages
  std::list<Message*> error_to_send;
  for (std::list<Message*>::iterator it = pending_messages_.begin();
       it != pending_messages_.end(); it++) {
    if (state_ != Socket::CONNECTED) {
      error_to_send.push_back(*it);
    } else {
      Error err = _Send(*it);
      if (err.Code() != LNR_OK) {
        error_to_send.push_back(*it);
      }
    }
  }
  std::list<Message*>().swap(pending_messages_);
  state_lock.unlock();

  // call OnError when fail to send pending messages
  Error pending_err = Error(LNR_ECANCELED);
  for (std::list<Message*>::iterator it = error_to_send.begin();
       it != error_to_send.end(); it++) {
    Message* message = *it;
    observer_->Lock();
    HandlerDelegate* delegate = observer_->GetSubject();
    if (delegate) {
      Socket socket = delegate->Get(id_); // copy socket
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
      observer_->Unlock();
    }
    delete *it;
  }
}

void SocketImpl::_DiscardMessages(const Socket& socket) {
  Error err = Error(LNR_ECANCELED);

  std::list<Message*> cancelled_pendings;
  unique_lock<mutex> state_lock(state_mutex_);
  cancelled_pendings.swap(pending_messages_);
  state_lock.unlock();
  for (std::list<Message*>::iterator it = cancelled_pendings.begin();
       it != cancelled_pendings.end(); it++) {
    Message* message = *it;
    observer_->Lock();
    HandlerDelegate* delegate = observer_->GetSubject();
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
    observer_->Unlock();
    delete *it;
  }

  std::list<RequestTimer*> cancelled_requests;
  unique_lock<mutex> request_timer_lock(request_timer_mutex_);
  for (std::list<RequestTimer*>::iterator it = request_timers_.begin();
       it != request_timers_.end(); it++) {
    (*it)->Stop();
  }
  cancelled_requests.swap(request_timers_);
  request_timer_lock.unlock();
  for (std::list<RequestTimer*>::iterator it = cancelled_requests.begin();
       it != cancelled_requests.end(); it++) {
    const Request& request = (*it)->GetRequest();
    observer_->Lock();
    HandlerDelegate* delegate = observer_->GetSubject();
    if (delegate) {
      delegate->OnError(socket, request, err);
    }
    observer_->Unlock();
    delete *it;
  }
}

}  // namespace linear
