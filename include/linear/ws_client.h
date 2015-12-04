/**
 * @file ws_client.h
 * WSClient class definition
 */

#ifndef LINEAR_WS_CLIENT_H_
#define LINEAR_WS_CLIENT_H_

#include "linear/client.h"
#include "linear/handler.h"
#include "linear/ws_socket.h"

namespace linear {

/**
 * @class WSClient ws_client.h "linear/ws_client.h"
 * WSClient class that extends Client class
 * @includelineno ws_client_sample.cpp
 */
class LINEAR_EXTERN WSClient : public Client {
 public:
  /// @cond hidden
  WSClient() : Client() {}
  virtual ~WSClient();
  /// @endcond
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] [request_context] common linear::WSRequestContext object
   */
  WSClient(const linear::Handler& handler,
           const linear::WSRequestContext& request_context = linear::WSRequestContext());
  /**
   * Set common linear::WSRequestContext into Client Object.
   * If you can not provide linear::WSRequestContext when construct WSClient, call this method.
   * @param [in] request_context linear::WSRequestContext object
   */
  void SetWSRequestContext(const linear::WSRequestContext& request_context);
  /**
   * Create new linear::WSSocket Object with common linear::WSRequestContext.
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   */
  linear::WSSocket CreateSocket(const std::string& hostname, int port);
  /**
   * Create new linear::WSSocket Object with linear::WSRequestContext differ from common context.
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   * @param [in] request_context request context for websocket.
   */
  linear::WSSocket CreateSocket(const std::string& hostname, int port, const linear::WSRequestContext& request_context);
};

}  // namespace linear

#endif  // LINEAR_WS_CLIENT_H_
