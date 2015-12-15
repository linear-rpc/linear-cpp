#include "linear/wss_server.h"

#include "wss_server_impl.h"

using namespace linear::log;

namespace linear {

WSSServer::WSSServer(const Handler& handler,
                     const SSLContext& ssl_context,
                     linear::AuthContext::Type type,
                     const std::string& realm,
                     const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new WSSServerImpl(handler, ssl_context, type, realm, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSSServer::WSSServer(const Handler& handler,
                     const SSLContext& ssl_context,
                     const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new WSSServerImpl(handler, ssl_context,
                                                       linear::AuthContext::UNUSED, "",
                                                       loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSSServer::WSSServer(const Handler& handler, const linear::EventLoop& loop) {
  try {
    server_ = shared_ptr<ServerImpl>(new WSSServerImpl(handler, linear::SSLContext(),
                                                       linear::AuthContext::UNUSED, "",
                                                       loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

void WSSServer::SetSSLContext(const SSLContext& ssl_context) {
  if (server_) {
    static_pointer_cast<WSSServerImpl>(server_)->SetSSLContext(ssl_context);
  }
}

void WSSServer::UseAuthentication(linear::AuthContext::Type auth_type, const std::string& realm) {
  if (server_) {
    static_pointer_cast<WSSServerImpl>(server_)->UseAuthentication(auth_type, realm);
  }
}

}  // namespace linear
