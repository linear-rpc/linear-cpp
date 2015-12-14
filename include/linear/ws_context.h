/**
 * @file ws_context.h
 * WSContext class definition
 **/

#ifndef LINEAR_WS_CONTEXT_H_
#define LINEAR_WS_CONTEXT_H_

#include "linear/auth_context.h"

//! 101 Switching Protocols
#define LNR_WS_OK                    (101)
//! 400 Bad Request
#define LNR_WS_BAD_REQUEST           (400)
//! 401 Unauthorized
#define LNR_WS_UNAUTHORIZED          (401)
//! 403 Forbidden
#define LNR_WS_FORBIDDEN             (403)
//! 404 Not Found
#define LNR_WS_NOT_FOUND             (404)
//! 500 Internal Server Error
#define LNR_WS_INTERNAL_SERVER_ERROR (500)
//! 503 Service Unavailable
#define LNR_WS_SERVICE_UNAVAILABLE   (503)

namespace linear {

/**
 * @class WSRequestContext ws_context.h "linear/ws_context.h"
 * WSRequestContext struct
 */
struct LINEAR_EXTERN WSRequestContext {
  /// @cond hidden
  WSRequestContext() : path("/"), query(), headers() {}
  ~WSRequestContext() {}
  /// @endcond
  std::string path;                           //!< path field of URL
  std::string query;                          //!< query string
  /**
   * additional request headers
   * @note
   * basic websocket headers are created automatically
   **/
  std::map<std::string, std::string> headers;
  /**
   * authentication infos
   * @see linear::AuthenticateContext
   */
  linear::AuthenticateContext authenticate;
  /**
   * authorization infos
   * @see linear::AuthorizationContext
   */
  linear::AuthorizationContext authorization;
};

/**
 * @class WSResponseContext ws_context.h "linear/ws_context.h"
 * WSResponseContext
 */
struct LINEAR_EXTERN WSResponseContext {
  /// @cond hidden
  WSResponseContext() : code(LNR_WS_OK), headers() {}
  ~WSResponseContext() {}
  /// @endcond
  int code; //!< http status code
  /**
   * additional response headers
   * @note
   * basic websocket headers are created automatically
   **/
  std::map<std::string, std::string> headers;
};

} // namespace linear

#endif // LINEAR_WS_CONTEXT_H_
