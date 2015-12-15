#include <cstdlib>
#include <sstream>

#include "linear/tcp_socket.h"

#include "event_loop_impl.h"
#include "tcp_server_impl.h"
#include "tcp_socket_impl.h"

using namespace linear::log;

namespace linear {

TCPServerImpl::TCPServerImpl(const Handler& handler, const linear::EventLoop& loop)
  : ServerImpl(handler, loop),
    handle_(NULL) {
}

TCPServerImpl::~TCPServerImpl() {
  Stop();
}

Error TCPServerImpl::Start(const std::string& hostname, int port, EventLoopImpl::ServerEvent* ev) {
  lock_guard<mutex> lock(mutex_);
  if (state_ == START) {
    return Error(LNR_EALREADY);
  }
  handle_ = static_cast<tv_tcp_t*>(malloc(sizeof(tv_tcp_t)));
  if (handle_ == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_tcp_init(loop_->GetHandle(), handle_);
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,TCP): %s",
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
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,TCP): %s",
               hostname.c_str(), port, err.Message().c_str());
    free(handle_);
    return err;
  }
  state_ = START;
  self_ = Addrinfo(hostname, port);
  LINEAR_LOG(LOG_DEBUG, "start server: %s:%d,TCP", self_.addr.c_str(), self_.port);
  return Error(LNR_OK);
}

Error TCPServerImpl::Stop() {
  lock_guard<mutex> lock(mutex_);
  if (state_ == STOP) {
    return Error(LNR_EALREADY);
  }
  LINEAR_LOG(LOG_DEBUG, "stop server: %s:%d,TCP", self_.addr.c_str(), self_.port);
  state_ = STOP;
  tv_close(reinterpret_cast<tv_handle_t*>(handle_), EventLoopImpl::OnClose);
  pool_.Clear();
  return Error(LNR_OK);
}

void TCPServerImpl::OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status) {
  unique_lock<mutex> lock(mutex_);
  if (state_ == STOP) {
    return;
  }
  assert(status || cli_stream != NULL);
  if (status) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,TCP, reason = %s",
               self_.addr.c_str(), self_.port,
               tv_strerror(reinterpret_cast<tv_handle_t*>(srv_stream), status));
    return;
  } else if (cli_stream == NULL) {
    // TODO: LNR_EINTENAL or LNR_ENOMEM?
    LINEAR_LOG(LOG_ERR, "BUG?: fail to accept at %s:%d,TCP, reason = Internal Server Error",
               self_.addr.c_str(), self_.port);
    return;
  }
  try {
    shared_ptr<TCPSocketImpl> shared = shared_ptr<TCPSocketImpl>(new TCPSocketImpl(cli_stream, loop_, *this));
    EventLoopImpl::SocketEvent* ev = new EventLoopImpl::SocketEvent(shared);
    if (shared->StartRead(ev) != Error(LNR_OK)) {
        throw std::runtime_error("fail to accept");
    }
    if (Retain(shared) == Error(LNR_ENOSPC)) {
      shared->Disconnect();
      return;
    }
    Group::Join(LINEAR_BROADCAST_GROUP, TCPSocket(shared));
    OnConnect(shared);
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,TCP, reason = %s",
               self_.addr.c_str(), self_.port, Error(LNR_ENOMEM).Message().c_str());
  }
}

}  // namespace linear
