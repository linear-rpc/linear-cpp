#include "linear/tcp_server.h"

#include "tcp_server_impl.h"

namespace linear {

TCPServer::TCPServer(const shared_ptr<Handler>& handler,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared now...
  server_ = shared_ptr<ServerImpl>(new TCPServerImpl(handler, loop));
}

}  // namespace linear
