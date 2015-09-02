#include "linear/tcp_socket.h"

#include "tcp_client_impl.h"
#include "tcp_socket_impl.h"

using namespace linear::log;

namespace linear {

TCPClientImpl::TCPClientImpl(const Handler& handler) : ClientImpl(handler) {
}

TCPClientImpl::~TCPClientImpl() {
}

TCPSocket TCPClientImpl::CreateSocket(const std::string& hostname, int port) {
  try {
    TCPSocket socket(shared_ptr<TCPSocketImpl>(new TCPSocketImpl(hostname, port, *this)));
    return socket;
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

}  // namespace linear
