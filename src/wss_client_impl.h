#ifndef LINEAR_WSS_CLIENT_IMPL_H_
#define LINEAR_WSS_CLIENT_IMPL_H_

#include "linear/wss_socket.h"
#include "linear/ws_context.h"
#include "linear/ssl_context.h"

#include "client_impl.h"

namespace linear {

class WSSClientImpl : public ClientImpl {
 public:
  WSSClientImpl(const linear::Handler& handler,
                const linear::WSRequestContext& request_context,
                const linear::SSLContext& ssl_context);
  virtual ~WSSClientImpl();
  void SetWSRequestContext(const linear::WSRequestContext& request_context);
  void SetSSLContext(const linear::SSLContext& ssl_context);
  linear::WSSSocket CreateSocket(const std::string& hostname, int port);
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::WSRequestContext& request_context);
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::WSRequestContext& request_context,
                                 const linear::SSLContext& ssl_context);
 private:
  linear::WSRequestContext request_context_;
  linear::SSLContext ssl_context_;
};

}

#endif // LINEAR_WSS_CLIENT_IMPL_H_
