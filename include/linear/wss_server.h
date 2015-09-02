/**
 * @file wss_server.h
 * WSSServer class definition
 */

#ifndef LINEAR_WSS_SERVER_H_
#define LINEAR_WSS_SERVER_H_

#include "linear/handler.h"
#include "linear/server.h"
#include "linear/wss_socket.h"
#include "linear/ssl_context.h"

namespace linear {

/**
 * @class WSSServer wss_server.h "linear/wss_server.h"
 *
 * WSSServer class that extends Server class
 * @includelineno wss_server_sample.cpp
 */
class LINEAR_EXTERN WSSServer : public Server {
 public:
  /// @cond hidden
  WSSServer() : Server() {}
  ~WSSServer();
  /// @endcond
  /**
   * WSSServer Constructor with SSLContext
   * @param [in] handler application defined behavior.
   * @param [in] ssl_context linear::SSLContext object
   * @param [in] auth_type authentication type
   * @param [in] realm authentication realm
   * @see linear::AuthContext
   */
  WSSServer(const linear::Handler& handler,
            const linear::SSLContext& ssl_context = linear::SSLContext(),
            linear::AuthContext::Type auth_type = linear::AuthContext::UNUSED,
            const std::string& realm = "Authorization Required");
  /**
   * Set SSLContext into Server Object.
   * If you can not provide handler when construct SSLServer, call this method.
   * @param [in] ssl_context linear::SSLContext object
   */
  void SetSSLContext(const linear::SSLContext& ssl_context);
};

}  // namespace linear

#endif  // LINEAR_WSS_SERVER_H_
