#ifndef LINEAR_WSS_SOCKET_IMPL_H_
#define LINEAR_WSS_SOCKET_IMPL_H_

#include "linear/ws_context.h"
#include "linear/ssl_context.h"

#include "auth_context_impl.h"
#include "socket_impl.h"

namespace linear {

class WSSSocketImpl : public linear::SocketImpl {
 public:
  // Client Socket
  WSSSocketImpl(const std::string& host, int port,
                const linear::WSRequestContext& ws_context, const SSLContext& ssl_context,
                const linear::HandlerDelegate& delegate);
  // Server Socket
  WSSSocketImpl(tv_stream_t* stream,
                const linear::WSRequestContext& ws_context, const SSLContext& ssl_context,
                const linear::HandlerDelegate& delegate);
  virtual ~WSSSocketImpl();

  linear::Error Connect();
  void OnConnect(tv_stream_t* handle, int status);
  bool CheckRetryAuth();
  const linear::WSRequestContext& GetWSRequestContext();
  void SetWSRequestContext(const WSRequestContext& request_context);
  const linear::WSResponseContext& GetWSResponseContext();
  void SetWSResponseContext(const WSResponseContext& response_context);
  linear::Error GetVerifyResult();
  bool PresentPeerCertificate();
  linear::X509Certificate GetPeerCertificate();

 private:
  WSRequestContext request_context_;
  WSResponseContext response_context_;
  linear::SSLContext ssl_context_;
  AuthenticateContextImpl authenticate_context_;
};

}  // namespace linear

#endif  // LINEAR_WSS_SOCKET_IMPL_H_
