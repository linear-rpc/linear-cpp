#ifndef LINEAR_TCP_CLIENT_IMPL_H_
#define LINEAR_TCP_CLIENT_IMPL_H_

#include "linear/tcp_socket.h"

#include "client_impl.h"

namespace linear {

class TCPClientImpl : public ClientImpl {
 public:
  explicit TCPClientImpl(const linear::Handler& handler);
  virtual ~TCPClientImpl();
  linear::TCPSocket CreateSocket(const std::string& hostname, int port);
};

}

#endif // LINEAR_TCP_CLIENT_IMPL_H_
