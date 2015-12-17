#include "linear/wss_server.h"

#include "wss_server_impl.h"

namespace linear {

WSSServer::WSSServer(const shared_ptr<Handler>& handler,
                     const SSLContext& ssl_context,
                     AuthContext::Type type,
                     const std::string& realm,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  server_ = shared_ptr<ServerImpl>(new WSSServerImpl(handler, ssl_context, type, realm, loop));
}

WSSServer::WSSServer(const shared_ptr<Handler>& handler,
                     const SSLContext& ssl_context,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  server_ = shared_ptr<ServerImpl>(new WSSServerImpl(handler, ssl_context, AuthContext::UNUSED, "", loop));
}

WSSServer::WSSServer(const shared_ptr<Handler>& handler, const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  server_ = shared_ptr<ServerImpl>(new WSSServerImpl(handler, SSLContext(), AuthContext::UNUSED, "", loop));
}

void WSSServer::SetSSLContext(const SSLContext& ssl_context) {
  if (server_) {
    static_pointer_cast<WSSServerImpl>(server_)->SetSSLContext(ssl_context);
  }
}

void WSSServer::UseAuthentication(AuthContext::Type auth_type, const std::string& realm) {
  if (server_) {
    static_pointer_cast<WSSServerImpl>(server_)->UseAuthentication(auth_type, realm);
  }
}

}  // namespace linear
