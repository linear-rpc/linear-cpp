#include "tcp_client_impl.h"
#include "tcp_socket_impl.h"

using namespace linear::log;

namespace linear {

TCPClientImpl::TCPClientImpl(const Handler& handler, const linear::EventLoop& loop)
  : ClientImpl(handler, loop) {
}

TCPSocket TCPClientImpl::CreateSocket(const std::string& hostname, int port) {
  try {
    return TCPSocket(shared_ptr<TCPSocketImpl>(new TCPSocketImpl(hostname, port, loop_, *this)));
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

}  // namespace linear
