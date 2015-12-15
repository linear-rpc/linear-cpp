#include "linear/tcp_client.h"

#include "tcp_client_impl.h"

using namespace linear::log;

namespace linear {

TCPClient::TCPClient(const Handler& handler, const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<TCPClientImpl>(new TCPClientImpl(handler, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

TCPSocket TCPClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<TCPClientImpl>(client_)->CreateSocket(hostname, port);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
