#include "linear/tcp_client.h"

#include "tcp_client_impl.h"

using namespace linear::log;

namespace linear {

TCPClient::TCPClient(const shared_ptr<Handler>& handler, const EventLoop& loop) {
  // TODO: we cannot use make_shared now...
  client_ = shared_ptr<TCPClientImpl>(new TCPClientImpl(handler, loop));
}

TCPSocket TCPClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<TCPClientImpl>(client_)->CreateSocket(hostname, port, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
