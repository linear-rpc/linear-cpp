/**
 * @file ssl_client.h
 * SSLClient class definition
 */

#ifndef LINEAR_SSL_CLIENT_H_
#define LINEAR_SSL_CLIENT_H_

#include "linear/client.h"
#include "linear/handler.h"
#include "linear/ssl_context.h"
#include "linear/ssl_socket.h"

namespace linear {

/**
 * @class SSLClient ssl_client.h "linear/ssl_client.h"
 * SSLClient class that extends Client class
 * @includelineno ssl_client_sample.cpp
 */
class LINEAR_EXTERN SSLClient : public Client {
 public:
  /// @cond hidden
  SSLClient() : Client() {}
  virtual ~SSLClient() {}
  /// @endcond
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] [context] common linear::SSLContext object
   * @param [in] [loop] eventloop(thread) object
   */
  SSLClient(const linear::shared_ptr<linear::Handler>& handler,
            const linear::SSLContext& context = linear::SSLContext(),
            const linear::EventLoop& loop = linear::EventLoop::GetDefault());
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] loop eventloop(thread) object
   */
  SSLClient(const linear::shared_ptr<linear::Handler>& handler,
            const linear::EventLoop& loop);
  /**
   * Set common linear::SSLContext into Client Object.
   * If you can not provide linear::SSLContext when construct SSLClient, call this method.
   * @param [in] context linear::SSLContext object
   */
  void SetSSLContext(const linear::SSLContext& context);
  /**
   * Create new linear::SSLSocket Object with common context
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   * @return linear::SSLSocket object
   */
  linear::SSLSocket CreateSocket(const std::string& hostname, int port);
  /**
   * Create new linear::SSLSocket Object with linear::SSLContext differ from common context
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   * @param [in] context linear::SSLContext object
   * @return linear::SSLSocket object
   */
  linear::SSLSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::SSLContext& context);
};

}  // namespace linear

#endif  // LINEAR_SSL_CLIENT_H_
