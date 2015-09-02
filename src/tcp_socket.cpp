#include "linear/log.h"
#include "linear/tcp_socket.h"

#include "tcp_socket_impl.h"

using namespace linear::log;

namespace linear {

TCPSocket::TCPSocket() : Socket() {
}

TCPSocket::TCPSocket(const shared_ptr<SocketImpl>& socket) : Socket(socket) {
  if (GetType() != Socket::TCP) {
    LINEAR_LOG(LOG_ERR, "invalid type_cast: type = %d, id = %d", GetType(), GetId());
    throw std::bad_cast();
  }
}

TCPSocket::TCPSocket(const shared_ptr<TCPSocketImpl>& tcp_socket) : Socket(tcp_socket) {
}

TCPSocket::~TCPSocket() {
}

}  // namespace linear
