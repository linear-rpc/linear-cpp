#ifndef LINEAR_SOCKET_IMPL_H_
#define LINEAR_SOCKET_IMPL_H_

#include "linear/message.h"
#include "linear/mutex.h"
#include "linear/timer.h"

#include "event_loop_impl.h"

namespace linear {

class HandlerDelegate;

class SocketImpl {
 public:
  class RequestTimer {
   public:
    RequestTimer(const linear::Request& r, const linear::weak_ptr<linear::SocketImpl> s)
      : request(r), socket(s) {}
    ~RequestTimer() {
      Stop();
    }
    void Start() {
      timer.Start(linear::EventLoopImpl::OnRequestTimeout, static_cast<uint64_t>(request.timeout), this);
    }
    void Stop() {
      timer.Stop();
    }
   public:
    linear::Request request;
    linear::weak_ptr<linear::SocketImpl> socket;
    linear::Timer timer;
  };
  
 public:
  // Client Socket
  SocketImpl(const std::string& host, int port,
             const linear::shared_ptr<linear::EventLoopImpl>& loop,
             const linear::weak_ptr<linear::HandlerDelegate>& delegate,
             linear::Socket::Type type);
  // Server Socket
  SocketImpl(tv_stream_t* stream,
             const linear::shared_ptr<linear::EventLoopImpl>& loop,
             const linear::weak_ptr<linear::HandlerDelegate>& delegate,
             linear::Socket::Type type);
  virtual ~SocketImpl();

  inline int GetId() { return id_; }
  inline linear::Socket::Type GetType() { return type_; }
  inline linear::Socket::State GetState() { return state_; }
  inline const linear::Addrinfo& GetSelfInfo() { return self_; }
  inline const linear::Addrinfo& GetPeerInfo() { return peer_; }

  void SetMaxBufferSize(size_t limit);
  linear::Error Connect(unsigned int timeout, linear::EventLoopImpl::SocketEvent* ev);
  linear::Error Disconnect(bool handshaking = false);
  linear::Error Send(const linear::Message& message, int timeout);
  linear::Error KeepAlive(unsigned int interval, unsigned int retry, Socket::KeepAliveType type);
  linear::Error BindToDevice(const std::string& ifname);
  linear::Error SetSockOpt(int level, int optname, const void* optval, size_t optlen);
  linear::Error StartRead(linear::EventLoopImpl::SocketEvent* ev);

  virtual void OnConnect(const shared_ptr<SocketImpl>& socket, tv_stream_t* handle, int status);
  void OnHandshakeComplete(const shared_ptr<SocketImpl>& socket, tv_stream_t* handle, int status);
  void OnDisconnect(const shared_ptr<SocketImpl>& socket);
  void OnRead(const shared_ptr<SocketImpl>& socket, const tv_buf_t *buffer, ssize_t nread);
  void OnWrite(const shared_ptr<SocketImpl>& socket, const linear::Message* message, int status);
  void OnConnectTimeout(const shared_ptr<SocketImpl>& socket);
  void OnRequestTimeout(const shared_ptr<SocketImpl>& socket, const linear::Request& request);

 protected:
  virtual linear::Error Connect() = 0;

  linear::Socket::State state_;
  tv_stream_t* stream_;
  linear::EventLoopImpl::SocketEvent* ev_;
  linear::Addrinfo self_, peer_;
  std::string bind_ifname_;
  linear::mutex state_mutex_;
  linear::shared_ptr<linear::EventLoopImpl> loop_;

 private:
  linear::Error _Send(linear::Message* ctx);
  void _SendPendingMessages(const shared_ptr<SocketImpl>& socket);
  void _DiscardMessages(const shared_ptr<SocketImpl>& socket);

  linear::Socket::Type type_;
  int id_;
  bool connectable_;
  bool handshaking_;
  linear::Error last_error_;
  linear::weak_ptr<linear::HandlerDelegate> delegate_;
  int connect_timeout_;
  linear::Timer connect_timer_;
  std::vector<linear::Message*> pending_messages_;
  std::vector<linear::SocketImpl::RequestTimer*> request_timers_;
  linear::mutex request_timer_mutex_;
  size_t max_buffer_size_;
  msgpack::unpacker unpacker_;
};

}  // namespace linear

#endif  // LINEAR_SOCKET_IMPL_H_
