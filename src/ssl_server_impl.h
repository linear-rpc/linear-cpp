#ifndef LINEAR_SSL_SERVER_IMPL_H_
#define LINEAR_SSL_SERVER_IMPL_H_

#include "linear/ssl_context.h"

#include "server_impl.h"

namespace linear {

class SSLServerImpl : public ServerImpl {
 public:
  SSLServerImpl(const linear::Handler& handler,
                const linear::SSLContext& context,
                const linear::EventLoop& loop);
  virtual ~SSLServerImpl();
  linear::Error Start(const std::string& hostname, int port,
                      linear::EventLoopImpl::ServerEvent* ev);
  linear::Error Stop();
  void OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status);
  void SetSSLContext(const linear::SSLContext& context) {
    context_ = context;
  }

 private:
  linear::SSLContext context_;
  tv_ssl_t* handle_;
};

}  // namespace linear

#endif  // LINEAR_SSL_SERVER_IMPL_H_
