/**
 * @file wss_client.h
 * WSSClient class definition
 **/

#ifndef LINEAR_WSS_CLIENT_H_
#define LINEAR_WSS_CLIENT_H_

#include "linear/client.h"
#include "linear/handler.h"
#include "linear/ssl_context.h"
#include "linear/wss_socket.h"

namespace linear {

/**
 * @class WSSClient wss_client.h "linear/wss_client.h"
 * WSSClient class that extends Client class
 * @includelineno wss_client_sample.cpp
 */
class LINEAR_EXTERN WSSClient : public Client {
 public:
  /// @cond hidden
  WSSClient() : Client() {}
  virtual ~WSSClient() {}
  /// @endcond
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] [request_context] common linear::WSRequestContext object
   * @param [in] [ssl_context] common linear::SSLContext object
   * @param [in] [loop] eventloop(thread) object
   */
  WSSClient(const linear::Handler& handler,
            const linear::WSRequestContext& request_context = linear::WSRequestContext(),
            const linear::SSLContext& ssl_context = linear::SSLContext(),
            const linear::EventLoop& loop = linear::EventLoop::GetDefault());
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] request_context common linear::WSRequestContext object
   * @param [in] loop eventloop(thread) object
   */
  WSSClient(const linear::Handler& handler,
            const linear::WSRequestContext& request_context,
            const linear::EventLoop& loop);
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] ssl_context common linear::SSLContext object
   * @param [in] [loop] eventloop(thread) object
   */
  WSSClient(const linear::Handler& handler,
            const linear::SSLContext& ssl_context,
            const linear::EventLoop& loop = linear::EventLoop::GetDefault());
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] loop eventloop(thread) object
   */
  WSSClient(const linear::Handler& handler, const linear::EventLoop& loop);
  /**
   * Set common linear::WSRequestContext into Client Object.
   * If you can not provide linear::WSRequestContext when construct WSSClient, call this method.
   * @param [in] request_context linear::WSRequestContext object
   */
  void SetWSRequestContext(const linear::WSRequestContext& request_context);
  /**
   * Set common linear::SSLContext into Client Object.
   * If you can not provide linear::SSLContext when construct WSSClient, call this method.
   * @param [in] ssl_context linear::SSLContext object
   */
  void SetSSLContext(const linear::SSLContext& ssl_context);
  /**
   * Create new linear::WSSSocket Object with common contexts
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   */
  linear::WSSSocket CreateSocket(const std::string& hostname, int port);
  /**
   * Create new linear::WSSSocket Object with linear::WSRequestContext differ from common linear::WSRequestContext.
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   * @param [in] request_context linear::WSRequestContext object
   */
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::WSRequestContext& request_context);
  /**
   * Create new linear::WSSSocket Object with linear::SSLContext differ from common linear::SSLContext.
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   * @param [in] ssl_context linear::SSLContext object
   */
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::SSLContext& ssl_context);
  /**
   * Create new linear::WSSSocket Object with linear::WSRequestContext and linear::SSLContext differ from common both.
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   * @param [in] request_context linear::WSRequestContext object
   * @param [in] ssl_context linear::SSLContext object
   */
  linear::WSSSocket CreateSocket(const std::string& hostname, int port,
                                 const linear::WSRequestContext& request_context,
                                 const linear::SSLContext& ssl_context);
};

}  // namespace linear

#endif  // LINEAR_WS_CLIENT_H_
