#include <cstdlib>

#include "server_impl.h"
#include "timer_impl.h"

using namespace linear::log;

namespace linear {

// -fthreadsafe-statics
const EventLoopImpl& EventLoopImpl::GetDefault() {
  static EventLoopImpl g_loop;
  return g_loop;
}

void EventLoopImpl::OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status) {
  assert(srv_stream != NULL && srv_stream->data != NULL);
  ServerEvent* ev = static_cast<ServerEvent*>(srv_stream->data);
  if (linear::shared_ptr<ServerImpl> server = ev->server.lock()) {
    server->OnAccept(srv_stream, cli_stream, status);
  }
}

void EventLoopImpl::OnAcceptComplete(tv_stream_t* stream, int status) {
  assert(stream != NULL && stream->data != NULL);
  SocketEvent* ev = static_cast<SocketEvent*>(stream->data);
  if (linear::shared_ptr<SocketImpl> socket = ev->socket.lock()) {
    socket->OnHandshakeComplete(socket, stream, status);
  }
}

void EventLoopImpl::OnConnect(tv_stream_t* stream, int status) {
  assert(stream != NULL && stream->data != NULL);
  SocketEvent* ev = static_cast<SocketEvent*>(stream->data);
  if (linear::shared_ptr<SocketImpl> socket = ev->socket.lock()) {
    socket->OnConnect(socket, stream, status);
  }
}

void EventLoopImpl::OnClose(tv_handle_t* handle) {
  assert(handle != NULL && handle->data != NULL);
  switch (static_cast<Event*>(handle->data)->type) {
  case SERVER:
    {
      ServerEvent* ev = static_cast<ServerEvent*>(handle->data);
      delete ev;
    }
    break;
  case SOCKET:
    {
      SocketEvent* ev = static_cast<SocketEvent*>(handle->data);
      if (linear::shared_ptr<SocketImpl> socket = ev->socket.lock()) {
        socket->OnDisconnect(socket);
      }
      delete ev;
    }
    break;
  case TIMER:
    {
      TimerEvent* ev = static_cast<TimerEvent*>(handle->data);
      delete ev;
    }
    break;
  default:
    LINEAR_LOG(LOG_ERR, "BUG: invalid type of event");
    assert(false);
  }
  free(handle);
}

void EventLoopImpl::OnRead(tv_stream_t* stream, ssize_t nread, const tv_buf_t* buffer) {
  assert(stream != NULL && stream->data != NULL && buffer != NULL);
  SocketEvent* ev = static_cast<SocketEvent*>(stream->data);
  if (linear::shared_ptr<SocketImpl> socket = ev->socket.lock()) {
    socket->OnRead(socket, buffer, nread);
  }
}

void EventLoopImpl::OnWrite(tv_write_t* request, int status) {
  assert(request != NULL && request->data != NULL &&
         request->handle != NULL && request->handle->data != NULL &&
         request->buf.base != NULL);
  Message* message = static_cast<Message*>(request->data);
  SocketEvent* ev = static_cast<SocketEvent*>(request->handle->data);
  if (linear::shared_ptr<SocketImpl> socket = ev->socket.lock()) {
    socket->OnWrite(socket, message, status);
  }
  delete message;
  free(request->buf.base);
  free(request);
}

void EventLoopImpl::OnTimer(tv_timer_t* handle) {
  assert(handle != NULL && handle->data != NULL);
  TimerEvent* ev = static_cast<TimerEvent*>(handle->data);
  if (linear::shared_ptr<TimerImpl> timer = ev->timer.lock()) {
    timer->OnTimer();
  }
}

void EventLoopImpl::OnConnectTimeout(void* args) {
  assert(args != NULL);
  SocketEvent* ev = static_cast<SocketEvent*>(args);
  if (linear::shared_ptr<SocketImpl> socket = ev->socket.lock()) {
    socket->OnConnectTimeout(socket);
  }
}

void EventLoopImpl::OnRequestTimeout(void* args) {
  assert(args != NULL);
  SocketImpl::RequestTimer* request_timer = static_cast<SocketImpl::RequestTimer*>(args);
  if (linear::shared_ptr<SocketImpl> socket = request_timer->socket.lock()) {
    socket->OnRequestTimeout(socket, request_timer->request);
  }
  delete request_timer;
}

EventLoopImpl::EventLoopImpl() : handle_(tv_loop_new()) {
  assert(handle_ != NULL);
}

EventLoopImpl::EventLoopImpl(const EventLoopImpl& loop) : handle_(loop.handle_) {
}

EventLoopImpl& EventLoopImpl::operator=(const EventLoopImpl& loop) {
  handle_ = loop.handle_;
  return *this;
}

EventLoopImpl::~EventLoopImpl() {
  tv_loop_delete(handle_);
}

tv_loop_t* EventLoopImpl::GetHandle() const {
  return handle_;
}

}  // namespace linear
