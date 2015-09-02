#include "linear/log.h"
#include "linear/tcp_server.h"

#include "tcp_server_impl.h"

using namespace linear::log;

namespace linear {

TCPServer::TCPServer(const Handler& handler) {
  try {
    server_ = shared_ptr<ServerImpl>(new TCPServerImpl(handler));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

TCPServer::~TCPServer() {
}

}  // namespace linear
