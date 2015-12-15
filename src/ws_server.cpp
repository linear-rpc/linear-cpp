#include "linear/ws_server.h"

#include "ws_server_impl.h"

using namespace linear::log;

namespace linear {

WSServer::WSServer(const Handler& handler,
                   linear::AuthContext::Type auth_type,
                   const std::string& realm,
                   const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new WSServerImpl(handler, auth_type, realm, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSServer::WSServer(const Handler& handler, const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new WSServerImpl(handler, linear::AuthContext::UNUSED, "", loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

void WSServer::UseAuthentication(linear::AuthContext::Type auth_type, const std::string& realm) {
  if (server_) {
    static_pointer_cast<WSServerImpl>(server_)->UseAuthentication(auth_type, realm);
  }
}

}  // namespace linear
