#include "linear/ws_server.h"

#include "ws_server_impl.h"

namespace linear {

WSServer::WSServer(const shared_ptr<Handler>& handler,
                   AuthContext::Type auth_type,
                   const std::string& realm,
                   const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  server_ = shared_ptr<ServerImpl>(new WSServerImpl(handler, auth_type, realm, loop));
}

WSServer::WSServer(const shared_ptr<Handler>& handler,
                   const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  server_ = shared_ptr<ServerImpl>(new WSServerImpl(handler, AuthContext::UNUSED, "", loop));
}

void WSServer::UseAuthentication(AuthContext::Type auth_type, const std::string& realm) {
  if (server_) {
    static_pointer_cast<WSServerImpl>(server_)->UseAuthentication(auth_type, realm);
  }
}

}  // namespace linear
