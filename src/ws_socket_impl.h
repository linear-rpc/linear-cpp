#ifndef LINEAR_WS_SOCKET_IMPL_H_
#define LINEAR_WS_SOCKET_IMPL_H_

#include "linear/ws_context.h"

#include "auth_context_impl.h"
#include "socket_impl.h"

namespace linear {

class WSSocketImpl : public linear::SocketImpl {
 public:
  // Client Socket
  WSSocketImpl(const std::string& host, int port,
               const linear::WSRequestContext& request_context,
               const linear::shared_ptr<linear::EventLoopImpl>& loop,
               const linear::HandlerDelegate& delegate);
  // Server Socket
  WSSocketImpl(tv_stream_t* stream, const linear::WSRequestContext& response_context,
               const linear::shared_ptr<linear::EventLoopImpl>& loop,
               const linear::HandlerDelegate& delegate);
  virtual ~WSSocketImpl();
  linear::Error Connect();
  void OnConnect(const shared_ptr<SocketImpl>& socket, tv_stream_t* handle, int status);
  bool CheckRetryAuth();
  const linear::WSRequestContext& GetWSRequestContext();
  void SetWSRequestContext(const WSRequestContext& request_context);
  const linear::WSResponseContext& GetWSResponseContext();
  void SetWSResponseContext(const WSResponseContext& response_context);

 private:
  WSRequestContext request_context_;
  WSResponseContext response_context_;
  AuthenticateContextImpl authenticate_context_;
};

}  // namespace linear

#endif  // LINEAR_WS_SOCKET_IMPL_H_
