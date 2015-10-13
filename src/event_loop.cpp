#include <assert.h>
#include <stdlib.h>

#include "linear/log.h"

#include "event_loop.h"
#include "server_impl.h"
#include "socket_impl.h"
#include "socket_pool.h"
#include "timer_impl.h"

using namespace linear::log;

namespace linear {

const EventLoop& EventLoop::GetDefault() {
  static EventLoop g_loop;
  return g_loop;
}

void EventLoop::AddTimer(const Timer& timer) {
  EventLoop& loop = const_cast<EventLoop&>(GetDefault());
  loop.pool_.Add(timer);
}

void EventLoop::RemoveTimer(int id) {
  EventLoop& loop = const_cast<EventLoop&>(GetDefault());
  loop.pool_.Remove(id);
}

void EventLoop::OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status) {
  assert(srv_stream != NULL && srv_stream->data != NULL);
  ServerEventData* data = static_cast<ServerEventData*>(srv_stream->data);
  data->Lock();
  ServerImpl* server = data->GetServer();
  if (server != NULL) {
    server->OnAccept(srv_stream, cli_stream, status);
  }
  data->Unlock();
}

void EventLoop::OnAcceptComplete(tv_stream_t* stream, int status) {
  assert(stream != NULL && stream->data != NULL);
  SocketEventData* data = static_cast<SocketEventData*>(stream->data);
  data->Lock();
  SocketImpl* socket = data->GetSocket();
  if (socket != NULL) {
    socket->OnHandshakeComplete(stream, status);
  }
  data->Unlock();
}

void EventLoop::OnConnect(tv_stream_t* stream, int status) {
  assert(stream != NULL && stream->data != NULL);
  SocketEventData* data = static_cast<SocketEventData*>(stream->data);
  data->Lock();
  SocketImpl* socket = data->GetSocket();
  if (socket != NULL) {
    socket->OnConnect(stream, status);
  }
  data->Unlock();
}

void EventLoop::OnClose(tv_handle_t* handle) {
  assert(handle != NULL);
  EventData* data = static_cast<EventData*>(handle->data);
  assert(data != NULL);
  switch (data->GetType()) {
  case SERVER_EVENT:
    {
      ServerEventData* server_event_data = static_cast<ServerEventData*>(handle->data);
      delete server_event_data;
      handle->data = NULL;
    }
    break;
  case SOCKET_EVENT:
    {
      linear::Socket socket; // need to copy socket to destruct after deleting socket event data.
      SocketEventData* socket_event_data = static_cast<SocketEventData*>(handle->data);
      socket_event_data->Lock();
      SocketImpl* socket_impl = socket_event_data->GetSocket();
      if (socket_impl != NULL) {
        socket = socket_impl->OnDisconnect();
      }
      socket_event_data->Unlock();

      EventLoop& loop = const_cast<EventLoop&>(GetDefault());
      loop.Lock();
      socket_event_data->Lock();
      socket_impl = socket_event_data->GetSocket();
      if (socket_impl != NULL) {
        socket_impl->UnrefResources();
      }
      socket_event_data->Unlock();
      loop.Unlock();

      delete socket_event_data;
    } // destruct socket here
    break;
  case TIMER_EVENT:
    {
      TimerEventData* timer_event_data = static_cast<TimerEventData*>(handle->data);
      RemoveTimer(timer_event_data->GetId());
      delete timer_event_data;
      handle->data = NULL;
    }
    break;
  default:
    break;
  }
  free(handle);
}

void EventLoop::OnRead(tv_stream_t* stream, ssize_t nread, const tv_buf_t* buffer) {
  assert(stream != NULL && stream->data != NULL && buffer != NULL);
  SocketEventData* data = static_cast<SocketEventData*>(stream->data);
  data->Lock();
  SocketImpl* socket = data->GetSocket();
  if (socket != NULL) {
    socket->OnRead(buffer, nread);
  }
  data->Unlock();
}

void EventLoop::OnWrite(tv_write_t* request, int status) {
  assert(request != NULL && request->handle != NULL &&
         request->handle->data != NULL && request->buf.base != NULL);
  if (status) {
    LINEAR_LOG(LOG_ERR, "fail to write: %s",
               tv_strerror(reinterpret_cast<tv_handle_t*>(request->handle), status));
  }
  SocketEventData* data = static_cast<SocketEventData*>(request->handle->data);
  const Message* message = static_cast<const Message*>(request->data);
  data->Lock();
  SocketImpl* socket = data->GetSocket();
  if (socket != NULL) {
    socket->OnWrite(message, status);
  } else {
    delete message;
  }
  data->Unlock();
  free(request->buf.base);
  free(request);
}

void EventLoop::OnConnectTimeout(void* args) {
  assert(args != NULL);
  SocketImpl* socket = static_cast<SocketImpl*>(args);
  socket->OnConnectTimeout();
}

void EventLoop::OnRequestTimeout(void* args) {
  SocketImpl::RequestTimer* timer = static_cast<SocketImpl::RequestTimer*>(args);
  assert(timer != NULL);
  SocketImpl* socket = timer->GetSocket();
  socket->OnRequestTimeout(timer->GetRequest());
  delete timer;
}

void EventLoop::OnTimer(tv_timer_t* handle) {
  assert(handle != NULL && handle->data != NULL);
  TimerEventData* data = static_cast<TimerEventData*>(handle->data);
  data->Lock();
  TimerImpl* timer = data->GetTimer();
  if (timer != NULL) {
    timer->OnTimer();
  }
  data->Unlock();
}

EventLoop::EventLoop() : handle_(tv_loop_new()) {
  assert(handle_ != NULL);
}

EventLoop::EventLoop(const EventLoop& loop) : handle_(loop.handle_) {
}

EventLoop& EventLoop::operator=(const EventLoop& loop) {
  handle_ = loop.handle_;
  return *this;
}

EventLoop::~EventLoop() {
  pool_.Clear();
  tv_loop_delete(handle_);
  handle_ = NULL;
}

void EventLoop::Lock() {
  mutex_.lock();
}

void EventLoop::Unlock() {
  mutex_.unlock();
}

tv_loop_t* EventLoop::GetHandle() const {
  return handle_;
}

}  // namespace linear
