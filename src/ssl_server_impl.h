#ifndef LINEAR_SSL_SERVER_IMPL_H_
#define LINEAR_SSL_SERVER_IMPL_H_

#include "linear/ssl_context.h"

#include "server_impl.h"

namespace linear {

class SSLServerImpl : public ServerImpl {
 public:
  SSLServerImpl(const linear::Handler& handler, const linear::SSLContext& context);
  virtual ~SSLServerImpl();
  void SetContext(const linear::SSLContext& context);
  linear::Error Start(const std::string& hostname, int port,
                      linear::EventLoopImpl::ServerEvent* ev);
  linear::Error Stop();
  void OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status);

 private:
  tv_ssl_t* handle_;
  linear::SSLContext context_;
};

}  // namespace linear

#endif  // LINEAR_SSL_SERVER_IMPL_H_
