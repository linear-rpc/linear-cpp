#ifndef LINEAR_SOCKET_IMPL_H_
#define LINEAR_SOCKET_IMPL_H_

#include "tv.h"

#include "linear/timer.h"

#include "event_loop.h"
#include "handler_delegate.h"

namespace linear {

class SocketImpl {
 public:
  class RequestTimer {
   public:
    RequestTimer(const linear::Request& request, linear::SocketImpl* socket)
      : request_(request), socket_(socket) {
    }
    ~RequestTimer() {
      Stop();
    }
    void Start() {
      timer_.Start(linear::EventLoop::OnRequestTimeout, static_cast<uint64_t>(request_.timeout), this);
    }
    void Stop() {
      timer_.Stop();
    }
    inline const linear::Request& GetRequest() {
      return request_;
    }
    inline linear::SocketImpl* GetSocket() {
      return socket_;
    }
   private:
    linear::Request request_;
    linear::SocketImpl* socket_;
    linear::Timer timer_;
  };
  
 public:
  // Client Socket
  SocketImpl(const std::string& host, int port, const linear::HandlerDelegate& delegate,
             linear::Socket::Type type);
  // Server Socket
  SocketImpl(tv_stream_t* stream, const linear::HandlerDelegate& delegate,
             linear::Socket::Type type);
  virtual ~SocketImpl();

  inline int GetId() { return id_; }
  inline linear::Socket::Type GetType() { return type_; }
  inline linear::Socket::State GetState() { return state_; }
  inline const linear::Addrinfo& GetSelfInfo() { return self_; }
  inline const linear::Addrinfo& GetPeerInfo() { return peer_; }

  void SetMaxBufferSize(size_t max_limit);
  linear::Error Connect(unsigned int timeout, const linear::Socket& socket);
  linear::Error Disconnect(bool handshaking = false);
  linear::Error Send(const linear::Message& message, int timeout);
  linear::Error KeepAlive(unsigned int interval, unsigned int retry);
  linear::Error BindToDevice(const std::string& ifname);
  linear::Error SetSockOpt(int level, int optname, const void* optval, size_t optlen);

  virtual void OnConnect(tv_stream_t* handle, int status);
  void OnHandshakeComplete(tv_stream_t* handle, int status);
  linear::Socket OnDisconnect();
  void OnRead(const tv_buf_t *buffer, ssize_t nread);
  void OnWrite(const linear::Message* message, int status);
  void OnConnectTimeout();
  void OnRequestTimeout(const linear::Request& request);
  void UnrefResources();

 protected:
  virtual linear::Error Connect() = 0;

  tv_stream_t* stream_;
  EventLoop::SocketEventData* data_;
  linear::Addrinfo self_, peer_;
  std::string bind_ifname_;

 private:
  linear::Error _Send(linear::Message* ctx);
  void _SendPendingMessages();
  void _DiscardMessages(const linear::Socket& socket);

  linear::Socket::Type type_;
  int id_;
  bool connectable_;
  bool handshaking_;
  linear::Error last_error_;
  linear::Socket::State state_;
  linear::shared_ptr<linear::Observer<linear::HandlerDelegate> > observer_;
  std::list<linear::Message*> pending_messages_;
  std::list<linear::SocketImpl::RequestTimer*> request_timers_;
  linear::mutex state_mutex_;
  linear::mutex request_timer_mutex_;
  linear::Timer connect_timer_;
  size_t max_buffer_size_;
  msgpack::unpacker unpacker_;
};

}  // namespace linear

#endif  // LINEAR_SOCKET_IMPL_H_
