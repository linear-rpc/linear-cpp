#include "linear/tcp_client.h"

#include "tcp_client_impl.h"

using namespace linear::log;

namespace linear {

TCPClient::TCPClient(const Handler& handler) {
  try {
    client_ = shared_ptr<TCPClientImpl>(new TCPClientImpl(handler));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

TCPClient::~TCPClient() {
}

TCPSocket TCPClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return dynamic_pointer_cast<TCPClientImpl>(client_)->CreateSocket(hostname, port);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
