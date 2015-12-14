#include <sstream>

#include "linear/log.h"

#include "ws_socket_impl.h"

using namespace linear::log;

namespace linear {

WSSocketImpl::WSSocketImpl(const std::string& host, int port, const WSRequestContext& request_context,
                           const HandlerDelegate& delegate)
  : SocketImpl(host, port, delegate, Socket::WS), request_context_(request_context) {
}

WSSocketImpl::WSSocketImpl(tv_stream_t* stream, const WSRequestContext& response_context,
                           const HandlerDelegate& delegate)
  : SocketImpl(stream, delegate, Socket::WS), request_context_(response_context) {
}

WSSocketImpl::~WSSocketImpl() {
}

Error WSSocketImpl::Connect() {
  tv_ws_t* handle = static_cast<tv_ws_t*>(malloc(sizeof(tv_ws_t)));
  if (handle == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_ws_init(EventLoopImpl::GetDefault().GetHandle(), handle);
  if (ret) {
    free(handle);
    return Error(ret);
  }
  std::string url;
  if (request_context_.path.compare(0, 1, "/") == 0) {
    url = request_context_.path;
  } else {
    url = "/" + request_context_.path;
  }
  if (!request_context_.query.empty()) {
    if (request_context_.query.compare(0, 1, "?") == 0 ) {
      url = url + request_context_.query;
    } else {
      url = url + "?" + request_context_.query;
    }
  }
  buffer_reset(&handle->handshake.request.url.raw);
  if (buffer_append(&handle->handshake.request.url.raw, url.c_str(), url.size())) {
    free(handle);
    return Error(LNR_ENOMEM);
  }
  if (request_context_.authenticate.type == AuthContext::BASIC || authenticate_context_.type == AuthContext::DIGEST) {
    std::string uri;
    if (request_context_.path.compare(0, 1, "/") == 0) {
      uri = request_context_.path;
    } else {
      uri = "/" + request_context_.path;
    }
    std::string authorization = authenticate_context_.CreateAuthorizationHeader(uri,
                                                                                request_context_.authenticate.username,
                                                                                request_context_.authenticate.password);
    if (!authorization.empty()) {
      LINEAR_LOG(LOG_DEBUG, "created Authorization: %s", authorization.c_str());
      request_context_.headers["Authorization"] = authorization;
    }
  } else if (authenticate_context_.type == AuthContext::UNUSED || authenticate_context_.type == AuthContext::UNKNOWN) {
    request_context_.headers.erase("Authorization");
  }
  buffer_kv kv;
  buffer_kv_init(&kv);
  for (std::map<std::string, std::string>::iterator it = request_context_.headers.begin();
       it != request_context_.headers.end(); it++) {
    buffer_kv_reset(&kv);
    if (buffer_append(&kv.key, it->first.c_str(), it->first.size()) ||
        buffer_append(&kv.val, it->second.c_str(), it->second.size()) ||
        buffer_kvs_insert(&handle->handshake.request.headers, &kv)) {
      buffer_kv_fin(&kv);
      free(handle);
      return Error(LNR_ENOMEM);
    }
  }
  buffer_kv_fin(&kv);
  if (!bind_ifname_.empty()) {
    ret = tv_bindtodevice(stream_, bind_ifname_.c_str());
    if (ret) {
      free(handle);
      return Error(ret);
    }
  }
  stream_ = reinterpret_cast<tv_stream_t*>(handle);
  stream_->data = ev_;
  response_context_.headers.clear(); // clear response context
  std::ostringstream port_str;
  port_str << peer_.port;
  ret = tv_connect(stream_, peer_.addr.c_str(), port_str.str().c_str(), EventLoopImpl::OnConnect);
  if (ret) {
    assert(false); // never reach now
    free(stream_);
    return Error(ret);
  }
  return Error(LNR_OK);
}

void WSSocketImpl::OnConnect(const shared_ptr<SocketImpl>& socket, tv_stream_t* stream, int status) {
  tv_ws_t* handle = reinterpret_cast<tv_ws_t*>(stream);

  response_context_.code = handle->handshake.response.code;
  for (const buffer_kv* kv = buffer_kvs_get_first(&handle->handshake.response.headers);
       kv; kv = buffer_kvs_get_next(kv)) {
    response_context_.headers[std::string(kv->key.ptr)] = std::string(kv->val.ptr);
  }
  const buffer* auth_val = buffer_kvs_case_find(&handle->handshake.response.headers, CONST_STRING("www-authenticate"));
  if (auth_val != NULL) {
    int nc = (authenticate_context_.nc > 0xfffd) ? 0 : authenticate_context_.nc; // for 32bit
    authenticate_context_ = AuthenticateContextImpl(std::string(auth_val->ptr, auth_val->len));
    authenticate_context_.nc = nc + 1;
  } else {
    authenticate_context_ = AuthenticateContextImpl();
  }
  SocketImpl::OnConnect(socket, stream, status);
}

bool WSSocketImpl::CheckRetryAuth() {
  return (response_context_.code == WSHS_UNAUTHORIZED &&
          authenticate_context_.type == AuthContext::DIGEST && authenticate_context_.nc < 2);
}

const linear::WSRequestContext& WSSocketImpl::GetWSRequestContext() {
  return request_context_;
}

void WSSocketImpl::SetWSRequestContext(const WSRequestContext& context) {
  request_context_ = context;
}

const linear::WSResponseContext& WSSocketImpl::GetWSResponseContext() {
  return response_context_;
}

void WSSocketImpl::SetWSResponseContext(const WSResponseContext& context) {
  response_context_ = context;
}

}  // namespace linear
