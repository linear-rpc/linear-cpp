#ifndef LINEAR_TCP_SOCKET_IMPL_H_
#define LINEAR_TCP_SOCKET_IMPL_H_

#include "socket_impl.h"

namespace linear {

class TCPSocketImpl : public linear::SocketImpl {
 public:
  // Client Socket
  TCPSocketImpl(const std::string& host, int port, const linear::HandlerDelegate& delegate);
  // Server Socket
  TCPSocketImpl(tv_stream_t* stream, const linear::HandlerDelegate& delegate);
  virtual ~TCPSocketImpl();

  linear::Error Connect();
};

}  // namespace linear

#endif  // LINEAR_TCP_SOCKET_IMPL_H_
