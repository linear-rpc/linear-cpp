#include "linear/ssl_server.h"

#include "ssl_server_impl.h"

using namespace linear::log;

namespace linear {

SSLServer::SSLServer(const Handler& handler,
                     const SSLContext& context,
                     const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new SSLServerImpl(handler, context, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

SSLServer::SSLServer(const Handler& handler, const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new SSLServerImpl(handler, linear::SSLContext(), loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

void SSLServer::SetSSLContext(const SSLContext& context) {
  if (server_) {
    static_pointer_cast<SSLServerImpl>(server_)->SetSSLContext(context);
  }
}

}  // namespace linear
