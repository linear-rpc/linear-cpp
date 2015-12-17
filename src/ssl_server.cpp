#include "linear/ssl_server.h"

#include "ssl_server_impl.h"

namespace linear {

SSLServer::SSLServer(const shared_ptr<Handler>& handler,
                     const SSLContext& context,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  server_ = shared_ptr<ServerImpl>(new SSLServerImpl(handler, context, loop));
}

SSLServer::SSLServer(const shared_ptr<Handler>& handler,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  server_ = shared_ptr<ServerImpl>(new SSLServerImpl(handler, SSLContext(), loop));
}

void SSLServer::SetSSLContext(const SSLContext& context) {
  if (server_) {
    static_pointer_cast<SSLServerImpl>(server_)->SetSSLContext(context);
  }
}

}  // namespace linear
