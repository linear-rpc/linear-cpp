#ifndef LINEAR_TCP_CLIENT_IMPL_H_
#define LINEAR_TCP_CLIENT_IMPL_H_

#include "linear/tcp_socket.h"

#include "client_impl.h"
#include "tcp_socket_impl.h"

namespace linear {

class TCPClientImpl : public ClientImpl {
 public:
  TCPClientImpl(const linear::weak_ptr<linear::Handler>& handler,
                const linear::EventLoop& loop)
    : ClientImpl(handler, loop) {}
  ~TCPClientImpl() {}
  linear::TCPSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::weak_ptr<linear::HandlerDelegate>& delegate) {
    return TCPSocket(shared_ptr<TCPSocketImpl>(new TCPSocketImpl(hostname, port, loop_, delegate)));
  }
};

}

#endif // LINEAR_TCP_CLIENT_IMPL_H_
