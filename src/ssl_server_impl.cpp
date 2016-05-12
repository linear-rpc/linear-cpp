#include <cstdlib>
#include <sstream>

#include "linear/ssl_socket.h"

#include "ssl_server_impl.h"
#include "ssl_socket_impl.h"

using namespace linear::log;

namespace linear {

SSLServerImpl::SSLServerImpl(const weak_ptr<Handler>& handler,
                             const SSLContext& context,
                             const EventLoop& loop)
  : ServerImpl(handler, loop, true),
    context_(context), handle_(NULL) {
}

SSLServerImpl::~SSLServerImpl() {
  Stop();
}

Error SSLServerImpl::Start(const std::string& hostname, int port, EventLoopImpl::ServerEvent* ev) {
  lock_guard<mutex> lock(mutex_);
  if (state_ == START) {
    return Error(LNR_EALREADY);
  }
  self_ = Addrinfo(hostname, port);
  if (self_.proto == Addrinfo::UNKNOWN) {
    Error err(LNR_EADDRNOTAVAIL);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               hostname.c_str(), port, err.Message().c_str());
    return err;
  }
  handle_ = static_cast<tv_ssl_t*>(malloc(sizeof(tv_ssl_t)));
  if (handle_ == NULL) {
    Error err(LNR_ENOMEM);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
               self_.port,
               err.Message().c_str());
    return err;
  }
  int ret = tv_ssl_init(loop_->GetHandle(), handle_, context_.GetHandle());
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
               self_.port,
               err.Message().c_str());
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
               (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
               self_.port,
               err.Message().c_str());
    free(handle_);
    return err;
  }
  state_ = START;
  LINEAR_LOG(LOG_DEBUG, "start server: %s:%d,SSL",
             (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
             self_.port);
  return Error(LNR_OK);
}

Error SSLServerImpl::Stop() {
  lock_guard<mutex> lock(mutex_);
  if (state_ == STOP) {
    return Error(LNR_EALREADY);
  }
  LINEAR_LOG(LOG_DEBUG, "stop server: %s:%d,SSL",
             (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
             self_.port);
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
               (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
               self_.port,
               tv_strerror(reinterpret_cast<tv_handle_t*>(srv_stream), status));
    return;
  } else if (cli_stream == NULL) {
    // TODO: LNR_EINTENAL or LNR_ENOMEM?
    LINEAR_LOG(LOG_ERR, "BUG?: fail to accept at %s:%d,SSL, reason = Internal Server Error",
               (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
               self_.port);
    return;
  }
  try {
    weak_ptr<HandlerDelegate> self = reinterpret_cast<EventLoopImpl::ServerEvent*>(handle_->data)->server;
    shared_ptr<SSLSocketImpl> shared = shared_ptr<SSLSocketImpl>(new SSLSocketImpl(cli_stream, context_, loop_, self));
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
               (self_.proto == Addrinfo::IPv4) ? self_.addr.c_str() : (std::string("[" + self_.addr + "]")).c_str(),
               self_.port,
               Error(LNR_ENOMEM).Message().c_str());
  }
}

}  // namespace linear
