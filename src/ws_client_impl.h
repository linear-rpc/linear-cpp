#ifndef LINEAR_WS_CLIENT_IMPL_H_
#define LINEAR_WS_CLIENT_IMPL_H_

#include "linear/ws_socket.h"

#include "client_impl.h"
#include "ws_socket_impl.h"

namespace linear {

class WSClientImpl : public ClientImpl {
 public:
  WSClientImpl(const linear::weak_ptr<linear::Handler>& handler,
               const linear::WSRequestContext& request_context,
               const linear::EventLoop& loop)
    : ClientImpl(handler, loop), request_context_(request_context) {}

  virtual ~WSClientImpl() {}
  void SetWSRequestContext(const linear::WSRequestContext& request_context) {
    request_context_ = request_context;
  }
  linear::WSSocket CreateSocket(const std::string& hostname, int port,
                                const linear::weak_ptr<HandlerDelegate>& delegate) {
    return CreateSocket(hostname, port, request_context_, delegate);
  }
  linear::WSSocket CreateSocket(const std::string& hostname, int port,
                                const linear::WSRequestContext& request_context,
                                const linear::weak_ptr<HandlerDelegate>& delegate) {
    return WSSocket(shared_ptr<WSSocketImpl>(new WSSocketImpl(hostname, port, request_context, loop_, delegate)));
  }
 private:
  linear::WSRequestContext request_context_;
};

}

#endif // LINEAR_WS_CLIENT_IMPL_H_
