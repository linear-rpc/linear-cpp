#ifndef LINEAR_WS_SERVER_IMPL_H_
#define LINEAR_WS_SERVER_IMPL_H_

#include "server_impl.h"
#include "nonce_pool.h"

namespace linear {

class WSServerImpl : public ServerImpl {
 public:
  WSServerImpl(const linear::Handler& handler, linear::AuthContext::Type auth_type, const std::string& realm);
  virtual ~WSServerImpl();
  linear::Error Start(const std::string& hostname, int port, linear::EventLoop::ServerEvent* ev);
  linear::Error Stop();
  void OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status);

 private:
  void CreateAuthenticationHeader(tv_ws_t* handle);

  NoncePool nonce_pool_;
  linear::AuthContext::Type auth_type_;
  std::string realm_;
  tv_ws_t* handle_;
};

}  // namespace linear

#endif  // LINEAR_WS_SERVER_IMPL_H_
