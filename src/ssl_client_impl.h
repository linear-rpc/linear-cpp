#ifndef LINEAR_SSL_CLIENT_IMPL_H_
#define LINEAR_SSL_CLIENT_IMPL_H_

#include "linear/ssl_socket.h"
#include "linear/ssl_context.h"

#include "client_impl.h"
#include "ssl_socket_impl.h"

namespace linear {

class SSLClientImpl : public ClientImpl {
 public:
  SSLClientImpl(const linear::weak_ptr<linear::Handler>& handler,
                const linear::SSLContext& context,
                const linear::EventLoop& loop)
    : ClientImpl(handler, loop, true), context_(context) {}
  ~SSLClientImpl() {}
  void SetSSLContext(const linear::SSLContext& context) {
    context_ = context;
  }
  linear::SSLSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::weak_ptr<linear::HandlerDelegate>& delegate) {
    return CreateSocket(hostname, port, context_, delegate);
  }
  linear::SSLSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::SSLContext& context,
                                 const linear::weak_ptr<linear::HandlerDelegate>& delegate) {
    return SSLSocket(shared_ptr<SSLSocketImpl>(new SSLSocketImpl(hostname, port, context, loop_, delegate)));
  }
 private:
  linear::SSLContext context_;
};

}

#endif // LINEAR_SSL_CLIENT_IMPL_H_
