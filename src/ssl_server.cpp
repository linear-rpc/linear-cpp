#include "linear/ssl_server.h"

#include "ssl_server_impl.h"

using namespace linear::log;

namespace linear {

SSLServer::SSLServer(const Handler& handler, const SSLContext& context) {
  try {
    server_ = shared_ptr<ServerImpl>(new SSLServerImpl(handler, context));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

SSLServer::~SSLServer() {
}

void SSLServer::SetContext(const SSLContext& context) {
  if (server_) {
    dynamic_pointer_cast<SSLServerImpl>(server_)->SetContext(context);
  }
}

}  // namespace linear
