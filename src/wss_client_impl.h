#ifndef LINEAR_WSS_CLIENT_IMPL_H_
#define LINEAR_WSS_CLIENT_IMPL_H_

#include "linear/wss_socket.h"
#include "linear/ssl_context.h"

#include "client_impl.h"
#include "wss_socket_impl.h"

namespace linear {

class WSSClientImpl : public ClientImpl {
 public:
  WSSClientImpl(const linear::weak_ptr<linear::Handler>& handler,
                const linear::WSRequestContext& request_context,
                const linear::SSLContext& ssl_context,
                const linear::EventLoop& loop)
      : ClientImpl(handler, loop, true),
        request_context_(request_context), ssl_context_(ssl_context) {}
  ~WSSClientImpl() {}
  void SetWSRequestContext(const linear::WSRequestContext& request_context) {
    request_context_ = request_context;
  }
  void SetSSLContext(const linear::SSLContext& ssl_context) {
    ssl_context_ = ssl_context;
  }
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::weak_ptr<HandlerDelegate>& delegate) {
    return CreateSocket(hostname, port, request_context_, ssl_context_, delegate);
  }
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::WSRequestContext& request_context,
                                 const linear::weak_ptr<HandlerDelegate>& delegate) {
    return CreateSocket(hostname, port, request_context, ssl_context_, delegate);
  }
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::SSLContext& ssl_context,
                                 const linear::weak_ptr<HandlerDelegate>& delegate) {
    return CreateSocket(hostname, port, request_context_, ssl_context, delegate);
  }
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::WSRequestContext& request_context,
                                 const linear::SSLContext& ssl_context,
                                 const linear::weak_ptr<HandlerDelegate>& delegate) {
    return WSSSocket(shared_ptr<WSSSocketImpl>(new WSSSocketImpl(hostname, port, request_context, ssl_context, loop_, delegate)));
  }
 private:
  linear::WSRequestContext request_context_;
  linear::SSLContext ssl_context_;
};

}

#endif // LINEAR_WSS_CLIENT_IMPL_H_
