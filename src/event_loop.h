/**
 * @file event_loop.h
 * Event loop class definition
 */

#ifndef LINEAR_EVENT_LOOP_H_
#define LINEAR_EVENT_LOOP_H_

#include "tv.h"

#include "linear/memory.h"

namespace linear {

class ServerImpl;
class SocketImpl;
class TimerImpl;

class EventLoop {
 public:
  enum EventType {
    SERVER,
    SOCKET,
    TIMER,
  };
  struct Event {
    Event(linear::EventLoop::EventType t) : type(t) {}
    virtual ~Event() {}
    linear::EventLoop::EventType type;
  };
  struct ServerEvent : public Event {
    ServerEvent(const linear::shared_ptr<linear::ServerImpl>& s)
      : Event(linear::EventLoop::SERVER), server(s) {}
    linear::weak_ptr<linear::ServerImpl> server;
  };
  struct SocketEvent : public Event {
    SocketEvent(const linear::shared_ptr<linear::SocketImpl>& s)
      : Event(linear::EventLoop::SOCKET), socket(s) {}
    linear::weak_ptr<linear::SocketImpl> socket;
  };
  struct TimerEvent : public Event {
    TimerEvent(const linear::shared_ptr<linear::TimerImpl>& t)
      : Event(linear::EventLoop::TIMER), timer(t) {}
    linear::weak_ptr<linear::TimerImpl> timer;
  };

 public:
  EventLoop();
  EventLoop(const EventLoop& loop);
  EventLoop& operator=(const EventLoop& loop);
  ~EventLoop();

  static const EventLoop& GetDefault();

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

#endif  // LINEAR_EVENT_LOOP_H_
