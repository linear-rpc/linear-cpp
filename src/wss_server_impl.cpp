#include <cstdlib>
#include <sstream>

#include "linear/log.h"
#include "linear/wss_socket.h"

#include "socket_pool.h"
#include "wss_server_impl.h"
#include "wss_socket_impl.h"

using namespace linear::log;

namespace linear {

WSSServerImpl::WSSServerImpl(const linear::Handler& handler, const linear::SSLContext& ssl_context, linear::AuthContext::Type auth_type, const std::string& realm)
  : ServerImpl(handler, true), auth_type_(auth_type), realm_(realm), ssl_context_(ssl_context), handle_(NULL) {
}

WSSServerImpl::~WSSServerImpl() {
  Stop();
}

void WSSServerImpl::SetSSLContext(const SSLContext& ssl_context) {
  ssl_context_ = ssl_context;
}

Error WSSServerImpl::Start(const std::string& hostname, int port, EventLoop::ServerEvent* ev) {
  lock_guard<mutex> lock(mutex_);
  if (state_ == START) {
    return Error(LNR_EALREADY);
  }
  handle_ = static_cast<tv_wss_t*>(malloc(sizeof(tv_wss_t)));
  if (handle_ == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_wss_init(EventLoop::GetDefault().GetHandle(), handle_, ssl_context_.GetHandle());
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,WSS): %s",
               self_.addr.c_str(), self_.port, err.Message().c_str());
    free(handle_);
    return err;
  }
  handle_->data = ev;
  std::ostringstream port_str;
  port_str << port;
  ret = tv_listen(reinterpret_cast<tv_stream_t*>(handle_),
                  hostname.c_str(), port_str.str().c_str(), ServerImpl::BACKLOG, EventLoop::OnAccept);
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,WSS): %s",
               hostname.c_str(), port, err.Message().c_str());
    free(handle_);
    return err;
  }
  state_ = START;
  self_ = Addrinfo(hostname, port);
  LINEAR_LOG(LOG_DEBUG, "start server: %s:%d,WSS", self_.addr.c_str(), self_.port);
  return Error(LNR_OK);
}

Error WSSServerImpl::Stop() {
  lock_guard<mutex> lock(mutex_);
  if (state_ == STOP) {
    return Error(LNR_EALREADY);
  }
  LINEAR_LOG(LOG_DEBUG, "stop server: %s:%d,WSS", self_.addr.c_str(), self_.port);
  state_ = STOP;
  tv_close(reinterpret_cast<tv_handle_t*>(handle_), EventLoop::OnClose);
  pool_.Clear();
  return Error(LNR_OK);
}

void WSSServerImpl::OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status) {
  unique_lock<mutex> lock(mutex_);
  if (state_ == STOP) {
    return;
  }
  assert(status || cli_stream != NULL);
  if (status) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,WSS, reason = %s",
               self_.addr.c_str(), self_.port,
               tv_strerror(reinterpret_cast<tv_handle_t*>(srv_stream), status));
    return;
  } else if (cli_stream == NULL) {
    // TODO: LNR_EINTENAL or LNR_ENOMEM?
    LINEAR_LOG(LOG_ERR, "BUG?: fail to accept at %s:%d,WSS, reason = Internal Server Error",
               self_.addr.c_str(), self_.port);
    return;
  }
  try {
    linear::WSRequestContext request_context_;
    shared_ptr<WSSSocketImpl> shared = shared_ptr<WSSSocketImpl>(new WSSSocketImpl(cli_stream, request_context_, ssl_context_, *this));
    EventLoop::SocketEvent* ev = new EventLoop::SocketEvent(shared);
    if (shared->StartRead(ev) != Error(LNR_OK)) {
        throw std::runtime_error("fail to accept");
    }
    // create WSRequestContext from handshake->request
    tv_wss_t* handle = (tv_wss_t*) cli_stream;
    if (handle->handshake.response.code == WSHS_SUCCESS) {
      if (handle->handshake.request.url.field_set & (1 << UF_PATH)) {
        request_context_.path = std::string(handle->handshake.request.url.field_value[UF_PATH].ptr);
      }
      if (handle->handshake.request.url.field_set & (1 << UF_QUERY)) {
        request_context_.query = std::string(handle->handshake.request.url.field_value[UF_QUERY].ptr);
      }
      for (const buffer_kv* kv = buffer_kvs_get_first(&handle->handshake.request.headers);
           kv; kv = buffer_kvs_get_next(kv)) {
        request_context_.headers[std::string(kv->key.ptr)] = std::string(kv->val.ptr);
      }
      if (Retain(shared) == Error(LNR_ENOSPC)) {
        handle->handshake.response.code = WSHS_SERVICE_UNAVAILABLE;
        return;
      }
      // check authorization header
      if (auth_type_ != AuthContext::UNUSED) {
        const buffer* auth_val = buffer_kvs_case_find(&handle->handshake.request.headers, CONST_STRING("authorization"));
        if (auth_val == NULL) { // not found Authorization header
          LINEAR_LOG(LOG_WARN, "need %s authentication at %s:%d,WS",
                     (auth_type_ == AuthContext::BASIC ? "basic" : "digest"), self_.addr.c_str(), self_.port);
          CreateAuthenticationHeader(handle);
          return;
        }
        AuthorizationContextImpl impl(std::string(auth_val->ptr, auth_val->len));
        if (auth_type_ != impl.type) {
          CreateAuthenticationHeader(handle);
          return;
        }
        if (auth_type_ == AuthContext::DIGEST) {
          impl.valid_nonce = nonce_pool_.IsValid(impl.nonce);
          nonce_pool_.Remove(impl.nonce);
        }
        AuthorizationContext authorization(linear::shared_ptr<AuthorizationContextImpl>(new AuthorizationContextImpl(impl)));
        authorization.type = auth_type_;
        authorization.username = impl.username;
        authorization.realm = impl.realm;
        request_context_.authorization = authorization;
        WSResponseContext response_context = shared->GetWSResponseContext();
        response_context.code = LNR_WS_UNAUTHORIZED;
        shared->SetWSResponseContext(response_context);
      }
      shared->SetWSRequestContext(request_context_);
      Group::Join(LINEAR_BROADCAST_GROUP, WSSSocket(shared));
      OnConnect(shared);

      // create handshake->response from WSResponseContext
      const WSResponseContext& response_context = shared->GetWSResponseContext();
      handle->handshake.response.code = static_cast<enum ws_handshake_response_code>(response_context.code);
      buffer_kv kv;
      buffer_kv_init(&kv);
      for (std::map<std::string, std::string>::const_iterator it = response_context.headers.begin();
           it != response_context.headers.end(); it++) {
        buffer_kv_reset(&kv);
        if (buffer_append(&kv.key, it->first.c_str(), it->first.size()) ||
            buffer_append(&kv.val, it->second.c_str(), it->second.size()) ||
            buffer_kvs_insert(&handle->handshake.response.headers, &kv)) {
          buffer_kv_fin(&kv);
          handle->handshake.response.code = WSHS_INTERNAL_SERVER_ERROR;
        }
      }
      buffer_kv_fin(&kv);
      if (auth_type_ != AuthContext::UNUSED &&
          handle->handshake.response.code == WSHS_UNAUTHORIZED &&
          buffer_kvs_case_find(&handle->handshake.response.headers, CONST_STRING("www-authenticate")) == NULL) {
        CreateAuthenticationHeader(handle);
      }
    } else {
      LINEAR_LOG(LOG_WARN, "fail to accept at %s:%d,WSS, reason = %s",
                 self_.addr.c_str(), self_.port, Error(LNR_EPROTO).Message().c_str());
    }
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,WSS, reason = %s",
               self_.addr.c_str(), self_.port, Error(LNR_ENOMEM).Message().c_str());
    reinterpret_cast<tv_wss_t*>(cli_stream)->handshake.response.code = WSHS_INTERNAL_SERVER_ERROR;
  }
}

void WSSServerImpl::CreateAuthenticationHeader(tv_wss_t* handle) {
  buffer_kv kv;
  buffer_kv_init(&kv);
  handle->handshake.response.code = WSHS_INTERNAL_SERVER_ERROR;
  if (buffer_append(&kv.key, CONST_STRING("WWW-Authenticate")) < 0) {
    buffer_kv_fin(&kv);
    return;
  }
  if (auth_type_  == AuthContext::BASIC) { // Basic
    if (buffer_append(&kv.val, CONST_STRING("Basic realm=\"")) < 0 ||
        buffer_append(&kv.val, realm_.c_str(), realm_.size()) < 0 ||
        buffer_append(&kv.val, CONST_STRING("\"")) < 0) {
      buffer_kv_fin(&kv);
      return;
    }
  } else { // Digest
    if (buffer_append(&kv.val, CONST_STRING("Digest realm=\"")) < 0 ||
        buffer_append(&kv.val, realm_.c_str(), realm_.size()) < 0 ||
        buffer_append(&kv.val, CONST_STRING("\", algorithm=MD5, qop=\"auth\", nonce=\"")) < 0) {
      buffer_kv_fin(&kv);
      return;
    }
    // create nonce: b64enc(24byte) => 32byte)
    buffer b;
    buffer_init(&b);
    Error e;
    do {
      if (buffer_fill_random(&b, 24) < 0 || buffer_to_base64(&b) < 0) {
        buffer_fin(&b);
        buffer_kv_fin(&kv);
        return;
      }
      e = nonce_pool_.Add(std::string(b.ptr, b.len)); // remove after 1 min by default
      if (e == Error(LNR_EALREADY)) {
        buffer_reset(&b);
      }
    } while(e == Error(LNR_EALREADY));
    if (e != Error(LNR_OK)) {
      buffer_fin(&b);
      buffer_kv_fin(&kv);
      return;
    }
    if (buffer_append(&kv.val, b.ptr, b.len) < 0) {
      buffer_fin(&b);
      buffer_kv_fin(&kv);
      return;
    }
    buffer_fin(&b);
    if (buffer_append(&kv.val, CONST_STRING("\"")) < 0) {
      buffer_kv_fin(&kv);
      return;
    }
  }
  if (buffer_kvs_insert(&handle->handshake.response.headers, &kv) < 0) {
    buffer_kv_fin(&kv);
    return;
  }
  buffer_kv_fin(&kv);
  handle->handshake.response.code = WSHS_UNAUTHORIZED;
  return;
}

}  // namespace linear
