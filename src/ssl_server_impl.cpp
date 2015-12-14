#include <cstdlib>
#include <sstream>

#include "linear/ssl_socket.h"

#include "ssl_server_impl.h"
#include "ssl_socket_impl.h"

using namespace linear::log;

namespace linear {

SSLServerImpl::SSLServerImpl(const Handler& handler, const SSLContext& context)
  : ServerImpl(handler, true), handle_(NULL), context_(context) {
}

SSLServerImpl::~SSLServerImpl() {
  Stop();
}

void SSLServerImpl::SetContext(const SSLContext& context) {
  context_ = context;
}

Error SSLServerImpl::Start(const std::string& hostname, int port, EventLoopImpl::ServerEvent* ev) {
  lock_guard<mutex> lock(mutex_);
  if (state_ == START) {
    return Error(LNR_EALREADY);
  }
  handle_ = static_cast<tv_ssl_t*>(malloc(sizeof(tv_ssl_t)));
  if (handle_ == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_ssl_init(EventLoopImpl::GetDefault().GetHandle(), handle_, context_.GetHandle());
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               hostname.c_str(), port, err.Message().c_str());
    free(handle_);
    return err;
  }
  handle_->data = ev;
  std::ostringstream port_str;
  port_str << port;
  ret = tv_listen(reinterpret_cast<tv_stream_t*>(handle_),
                  hostname.c_str(), port_str.str().c_str(), ServerImpl::BACKLOG, EventLoopImpl::OnAccept);
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               hostname.c_str(), port, err.Message().c_str());
    free(handle_);
    return err;
  }
  state_ = START;
  self_ = Addrinfo(hostname, port);
  LINEAR_LOG(LOG_DEBUG, "start server: %s:%d,SSL", self_.addr.c_str(), self_.port);
  return Error(LNR_OK);
}

Error SSLServerImpl::Stop() {
  lock_guard<mutex> lock(mutex_);
  if (state_ == STOP) {
    return Error(LNR_EALREADY);
  }
  LINEAR_LOG(LOG_DEBUG, "stop server: %s:%d,SSL", self_.addr.c_str(), self_.port);
  state_ = STOP;
  tv_close(reinterpret_cast<tv_handle_t*>(handle_), EventLoopImpl::OnClose);
  pool_.Clear();
  return Error(LNR_OK);
}

void SSLServerImpl::OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status) {
  unique_lock<mutex> lock(mutex_);
  if (state_ == STOP) {
    return;
  }
  assert(status || cli_stream != NULL);
  if (status) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,SSL, reason = %s",
               self_.addr.c_str(), self_.port,
               tv_strerror(reinterpret_cast<tv_handle_t*>(srv_stream), status));
    return;
  } else if (cli_stream == NULL) {
    // TODO: LNR_EINTENAL or LNR_ENOMEM?
    LINEAR_LOG(LOG_ERR, "BUG?: fail to accept at %s:%d,SSL, reason = Internal Server Error",
               self_.addr.c_str(), self_.port);
    return;
  }
  try {
    shared_ptr<SSLSocketImpl> shared = shared_ptr<SSLSocketImpl>(new SSLSocketImpl(cli_stream, context_, *this));
    EventLoopImpl::SocketEvent* ev = new EventLoopImpl::SocketEvent(shared);
    if (shared->StartRead(ev) != Error(LNR_OK)) {
        throw std::runtime_error("fail to accept");
    }
    if (Retain(shared) == Error(LNR_ENOSPC)) {
      shared->Disconnect();
      return;
    }
    Group::Join(LINEAR_BROADCAST_GROUP, SSLSocket(shared));
    OnConnect(shared);
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,SSL, reason = %s",
               self_.addr.c_str(), self_.port, Error(LNR_ENOMEM).Message().c_str());
  }
}

}  // namespace linear
