#include <stdlib.h>

#include <sstream>

#include "linear/log.h"
#include "linear/ssl_socket.h"

#include "socket_pool.h"
#include "ssl_server_impl.h"
#include "ssl_socket_impl.h"

using namespace linear::log;

namespace linear {

SSLServerImpl::SSLServerImpl(const Handler& handler)
  : ServerImpl(handler, true), handle_(NULL), context_() {
}

SSLServerImpl::SSLServerImpl(const Handler& handler, const SSLContext& context)
  : ServerImpl(handler, true), handle_(NULL), context_(context) {
}

SSLServerImpl::~SSLServerImpl() {
  Stop();
}

void SSLServerImpl::SetContext(const SSLContext& context) {
  context_ = context;
}

Error SSLServerImpl::Start(const std::string& hostname, int port) {
  lock_guard<mutex> lock(mutex_);

  if (handle_ != NULL) {
    Error err(LNR_EALREADY);
    LINEAR_LOG(LOG_WARN, "fail to start server(%s:%d,SSL): %s",
               self_.addr.c_str(), self_.port, err.Message().c_str());
    return err;
  }
  self_ = Addrinfo(hostname, port);
  handle_ = static_cast<tv_ssl_t*>(malloc(sizeof(tv_ssl_t)));
  if (handle_ == NULL) {
    Error err(LNR_ENOMEM);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               self_.addr.c_str(), self_.port, err.Message().c_str());
    return err;
  }
  int ret = tv_ssl_init(EventLoop::GetDefault().GetHandle(), handle_, context_.GetHandle());
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               self_.addr.c_str(), self_.port, err.Message().c_str());
    free(handle_);
    handle_ = NULL;
    return err;
  }
  EventLoop::ServerEventData* data = new EventLoop::ServerEventData();
  data->Register(this);
  handle_->data = data;
  std::ostringstream port_str;
  port_str << port;
  ret = tv_listen(reinterpret_cast<tv_stream_t*>(handle_),
                  hostname.c_str(), port_str.str().c_str(), ServerImpl::BACKLOG, EventLoop::OnAccept);
  if (ret) {
    Error err(ret);
    LINEAR_LOG(LOG_ERR, "fail to start server(%s:%d,SSL): %s",
               self_.addr.c_str(), self_.port, err.Message().c_str());
    free(handle_);
    handle_ = NULL;
    return err;
  }
  LINEAR_LOG(LOG_DEBUG, "start server: %s:%d,SSL", self_.addr.c_str(), self_.port);
  return Error(LNR_OK);
}

Error SSLServerImpl::Stop() {
  lock_guard<mutex> lock(mutex_);

  if (handle_ == NULL) {
    return Error(LNR_EALREADY);
  }
  EventLoop::ServerEventData* data = reinterpret_cast<EventLoop::ServerEventData*>(handle_->data);
  data->Lock();
  data->Unregister();
  data->Unlock();
  tv_close(reinterpret_cast<tv_handle_t*>(handle_), EventLoop::OnClose);
  LINEAR_LOG(LOG_DEBUG, "stop server: %s:%d,SSL", self_.addr.c_str(), self_.port);
  handle_ = NULL; // only dereferencing.delete at EventLoop::OnClose
  pool_.Clear();
  return Error(LNR_OK);
}

void SSLServerImpl::OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status) {
  assert(status || cli_stream != NULL);
  if (status) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,SSL, reason = %s",
               self_.addr.c_str(), self_.port, tv_strerror(reinterpret_cast<tv_handle_t*>(srv_stream), status));
    return;
  } else if (cli_stream == NULL) {
    // TODO: LNR_EINTENAL or LNR_ENOMEM?
    LINEAR_LOG(LOG_ERR, "BUG?: fail to accept at %s:%d,SSL, reason = Internal Server Error",
               self_.addr.c_str(), self_.port);
    return;
  }
  try {
    SSLSocket socket(shared_ptr<SSLSocketImpl>(new SSLSocketImpl(cli_stream, context_, *this)));
    if (Retain(socket).Code() == LNR_ENOSPC) {
      socket.Disconnect();
      return;
    }
    Group::Join(LINEAR_BROADCAST_GROUP, socket);
    OnConnect(socket.GetId());
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "fail to accept at %s:%d,SSL, reason = %s",
               self_.addr.c_str(), self_.port, Error(LNR_ENOMEM).Message().c_str());
  }
}

}  // namespace linear
