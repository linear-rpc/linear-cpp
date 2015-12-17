#ifndef LINEAR_WSS_SERVER_IMPL_H_
#define LINEAR_WSS_SERVER_IMPL_H_

#include "linear/ssl_context.h"

#include "server_impl.h"
#include "nonce_pool.h"

namespace linear {

class WSSServerImpl : public ServerImpl {
 public:
  WSSServerImpl(const linear::weak_ptr<linear::Handler>& handler,
                const linear::SSLContext& ssl_context,
                linear::AuthContext::Type auth_type,
                const std::string& realm,
                const linear::EventLoop& loop);
  virtual ~WSSServerImpl();
  linear::Error Start(const std::string& hostname, int port,
                      linear::EventLoopImpl::ServerEvent* ev);
  linear::Error Stop();
  void OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status);
  void SetSSLContext(const linear::SSLContext& ssl_context) {
    ssl_context_ = ssl_context;
  }
  void UseAuthentication(linear::AuthContext::Type auth_type, const std::string& realm) {
    auth_type_ = auth_type;
    realm_ = realm;
  }

 private:
  void CreateAuthenticationHeader(tv_wss_t* handle);

  NoncePool nonce_pool_;
  linear::AuthContext::Type auth_type_;
  std::string realm_;
  linear::SSLContext ssl_context_;
  tv_wss_t* handle_;
};

}  // namespace linear

#endif  // LINEAR_WSS_SERVER_IMPL_H_
