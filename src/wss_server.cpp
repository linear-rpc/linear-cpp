#include "linear/wss_server.h"

#include "wss_server_impl.h"

using namespace linear::log;

namespace linear {

WSSServer::WSSServer(const Handler& handler, const SSLContext& ssl_context, linear::AuthContext::Type type, const std::string& realm) {
  try {
    server_ = shared_ptr<ServerImpl>(new WSSServerImpl(handler, ssl_context, type, realm));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSSServer::~WSSServer() {
}

void WSSServer::SetSSLContext(const SSLContext& ssl_context) {
  if (server_) {
    dynamic_pointer_cast<WSSServerImpl>(server_)->SetSSLContext(ssl_context);
  }
}

}  // namespace linear
