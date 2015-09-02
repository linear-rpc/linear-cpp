/**
 * @file ws_socket.h
 * WSSocket class definition
 */

#ifndef LINEAR_WS_SOCKET_H_
#define LINEAR_WS_SOCKET_H_

#include "linear/socket.h"
#include "linear/ws_context.h"

namespace linear {

class WSSocketImpl;

/**
 * @class WSSocket ws_socket.h "linear/ws_socket.h"
 * WSSocket class that extends Socket class
 */
class LINEAR_EXTERN WSSocket : public Socket {
 public:
  /// @cond hidden
  WSSocket();
  explicit WSSocket(const linear::shared_ptr<linear::SocketImpl>& socket);
  explicit WSSocket(const linear::shared_ptr<linear::WSSocketImpl>& ws_socket);
  virtual ~WSSocket();
  /// @endcond
  /**
   * get http request context
   * @return linear::WSRequestContext object
   */
  const linear::WSRequestContext& GetWSRequestContext() const;
  /**
   * set http request context
   * @param [in] request_context http request context
   */
  void SetWSRequestContext(const WSRequestContext& request_context) const;
  /**
   * get response context
   * @return linear::WSResponseContext object
   */
  const linear::WSResponseContext& GetWSResponseContext() const;
  /**
   * set http response context
   * @param [in] response_context http response context
   */
  void SetWSResponseContext(const WSResponseContext& response_context) const;
};

}  // namespace linear

#endif  // LINEAR_WS_SOCKET_H_
