#include "linear/tcp_server.h"

#include "tcp_server_impl.h"

using namespace linear::log;

namespace linear {

TCPServer::TCPServer(const linear::Handler& handler, const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new TCPServerImpl(handler, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

}  // namespace linear
