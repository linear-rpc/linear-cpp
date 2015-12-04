#ifndef LINEAR_SSL_CLIENT_IMPL_H_
#define LINEAR_SSL_CLIENT_IMPL_H_

#include "linear/ssl_socket.h"
#include "linear/ssl_context.h"

#include "client_impl.h"

namespace linear {

class SSLClientImpl : public ClientImpl {
 public:
  SSLClientImpl(const linear::Handler& handler, const linear::SSLContext& context);
  virtual ~SSLClientImpl();
  void SetContext(const linear::SSLContext& context);
  linear::SSLSocket CreateSocket(const std::string& hostname, int port);
  linear::SSLSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::SSLContext& context);
 private:
  linear::SSLContext context_;
};

}

#endif // LINEAR_SSL_CLIENT_IMPL_H_
