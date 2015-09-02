/**
 * @file event_loop.h
 * Event loop class definition
 */

#ifndef LINEAR_EVENT_LOOP_H_
#define LINEAR_EVENT_LOOP_H_

#include "tv.h"

#include "linear/mutex.h"
#include "timer_pool.h"

namespace linear {

class ServerImpl;
class SocketImpl;
class TimerImpl;

class EventLoop {
 public:
  enum DataType {
    UNDEFINED,
    SERVER_EVENT,
    SOCKET_EVENT,
    TIMER_EVENT,
  };
  class EventData {
   public:
    EventData(linear::EventLoop::DataType type) : type_(type) {}
    virtual ~EventData() {
      type_ = UNDEFINED;
    }
    linear::EventLoop::DataType GetType() {
      return type_;
    }
    virtual void Lock() {
      mutex_.lock();
    }
    virtual void Unlock() {
      mutex_.unlock();
    }
   private:
    linear::EventLoop::DataType type_;
    linear::mutex mutex_;
  };
  class ServerEventData : public EventData {
   public:
    ServerEventData() : EventData(SERVER_EVENT), server_(NULL) {}
    ~ServerEventData() {
      Unregister();
    }
    linear::ServerImpl* GetServer() {
      return server_;
    }
    void Register(linear::ServerImpl* server) {
      server_ = server;
    }
    void Unregister() {
      server_ = NULL;
    }
   private:
    linear::ServerImpl* server_;
  };
  class SocketEventData : public EventData {
   public:
    SocketEventData() : EventData(SOCKET_EVENT), socket_(NULL) {}
    ~SocketEventData() {
      Unregister();
    }
    linear::SocketImpl* GetSocket() {
      return socket_;
    }
    void Register(linear::SocketImpl* socket) {
      socket_ = socket;
    }
    void Unregister() {
      socket_ = NULL;
    }
   private:
    linear::SocketImpl* socket_;
  };
  class TimerEventData : public EventData {
   public:
    TimerEventData(int id, linear::TimerImpl* timer) : EventData(TIMER_EVENT), id_(id), timer_(timer) {}
    ~TimerEventData() {
      timer_ = NULL;
    }
    int GetId() {
      return id_;
    }
    linear::TimerImpl* GetTimer() {
      return timer_;
    }
    void Unregister() {
      timer_ = NULL;
    }
   private:
    int id_;
    linear::TimerImpl* timer_;
  };

 public:
  EventLoop();
  EventLoop(const EventLoop& loop);
  EventLoop& operator=(const EventLoop& loop);
  ~EventLoop();
  void Lock();
  void Unlock();

  static const EventLoop& GetDefault();
  static void AddTimer(const linear::Timer& timer);
  static void RemoveTimer(int id);

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
  linear::TimerPool pool_;
  tv_loop_t* handle_;
  linear::mutex mutex_;
};

}  // namespace linear

#endif  // LINEAR_EVENT_LOOP_H_
