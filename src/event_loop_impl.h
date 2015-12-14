/**
 * @file event_loop_impl.h
 * Event loop class definition
 */

#ifndef LINEAR_EVENT_LOOP_IMPL_H_
#define LINEAR_EVENT_LOOP_IMPL_H_

#include "tv.h"

#include "linear/memory.h"

namespace linear {

class ServerImpl;
class SocketImpl;
class TimerImpl;

class EventLoopImpl {
 public:
  enum EventType {
    SERVER,
    SOCKET,
    TIMER,
  };
  struct Event {
    Event(linear::EventLoopImpl::EventType t) : type(t) {}
    virtual ~Event() {}
    linear::EventLoopImpl::EventType type;
  };
  struct ServerEvent : public Event {
    ServerEvent(const linear::shared_ptr<linear::ServerImpl>& s)
      : Event(linear::EventLoopImpl::SERVER), server(s) {}
    linear::weak_ptr<linear::ServerImpl> server;
  };
  struct SocketEvent : public Event {
    SocketEvent(const linear::shared_ptr<linear::SocketImpl>& s)
      : Event(linear::EventLoopImpl::SOCKET), socket(s) {}
    linear::weak_ptr<linear::SocketImpl> socket;
  };
  struct TimerEvent : public Event {
    TimerEvent(const linear::shared_ptr<linear::TimerImpl>& t)
      : Event(linear::EventLoopImpl::TIMER), timer(t) {}
    linear::weak_ptr<linear::TimerImpl> timer;
  };

 public:
  EventLoopImpl();
  EventLoopImpl(const EventLoopImpl& loop);
  EventLoopImpl& operator=(const EventLoopImpl& loop);
  ~EventLoopImpl();

  static const EventLoopImpl& GetDefault();

  static void OnAccept(tv_stream_t* server, tv_stream_t* client, int status);
  static void OnAcceptComplete(tv_stream_t* stream, int status);
  static void OnConnect(tv_stream_t* handle, int status);
  static void OnClose(tv_handle_t* handle);
  static void OnRead(tv_stream_t* handle, ssize_t nread, const tv_buf_t* buf);
  static void OnWrite(tv_write_t* req, int status);
  static void OnTimer(tv_timer_t* tv_timer);

  static void OnConnectTimeout(void* args);
  static void OnRequestTimeout(void* args);

  tv_loop_t* GetHandle() const;

 private:
  tv_loop_t* handle_;
};

}  // namespace linear

#endif  // LINEAR_EVENT_LOOP_IMPL_H_
