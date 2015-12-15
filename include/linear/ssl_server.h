/**
 * @file ssl_server.h
 * SSLServer class definition
 **/

#ifndef LINEAR_SSL_SERVER_H_
#define LINEAR_SSL_SERVER_H_

#include "linear/handler.h"
#include "linear/server.h"
#include "linear/ssl_context.h"
#include "linear/ssl_socket.h"

namespace linear {

/**
 * @class SSLServer ssl_server.h "linear/ssl_server.h"
 *
 * SSLServer class that extends Server class
 * @includelineno ssl_server_sample.cpp
 */
class LINEAR_EXTERN SSLServer : public Server {
 public:
  /// @cond hidden
  SSLServer() : Server() {}
  virtual ~SSLServer() {}
  /// @endcond
  /**
   * SSLServer Constructor
   * @param [in] handler application defined behavior.
   * @param [in] [context] linear::SSLContext object
   */
  SSLServer(const linear::Handler& handler,
            const linear::SSLContext& context = linear::SSLContext(),
            const linear::EventLoop& loop = linear::EventLoop::GetDefault());
  /**
   * SSLServer Constructor
   * @param [in] handler application defined behavior.
   * @param [in] [loop] eventloop(thread) object.
   */
  SSLServer(const linear::Handler& handler, const linear::EventLoop& loop);
  /**
   * Set SSLContext into Server Object.
   * If you can not provide handler when construct SSLServer, call this method.
   * @param [in] context linear::SSLContext object
   */
  void SetSSLContext(const linear::SSLContext& context);
};

}  // namespace linear

#endif  // LINEAR_SSL_SERVER_H_
