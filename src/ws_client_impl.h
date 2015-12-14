#ifndef LINEAR_WS_CLIENT_IMPL_H_
#define LINEAR_WS_CLIENT_IMPL_H_

#include "linear/ws_socket.h"

#include "client_impl.h"

namespace linear {

class WSClientImpl : public ClientImpl {
 public:
  WSClientImpl(const linear::Handler& handler, const linear::WSRequestContext& request_context);
  virtual ~WSClientImpl();
  void SetWSRequestContext(const linear::WSRequestContext& request_context);
  linear::WSSocket CreateSocket(const std::string& hostname, int port);
  linear::WSSocket CreateSocket(const std::string& hostname, int port,
                                const linear::WSRequestContext& request_context);
 private:
  linear::WSRequestContext request_context_;
};

}

#endif // LINEAR_WS_CLIENT_IMPL_H_
