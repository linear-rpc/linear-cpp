#include "linear/ws_server.h"

#include "ws_server_impl.h"

using namespace linear::log;

namespace linear {

WSServer::WSServer(const Handler& handler,
                   linear::AuthContext::Type auth_type,
                   const std::string& realm) {
  try {
    server_ = shared_ptr<ServerImpl>(new WSServerImpl(handler, auth_type, realm));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSServer::~WSServer() {
}

}  // namespace linear
