/**
 * @file wss_socket.h
 * WSSSocket class definition
 */

#ifndef LINEAR_WSS_SOCKET_H_
#define LINEAR_WSS_SOCKET_H_

#include "linear/socket.h"
#include "linear/ws_context.h"
#include "linear/x509_certificate.h"

namespace linear {

class WSSSocketImpl;

/**
 * @class WSSSocket wss_socket.h "linear/wss_socket.h"
 * WSSSocket class that extends Socket class
 */
class LINEAR_EXTERN WSSSocket : public Socket {
 public:
  /// @cond hidden
  WSSSocket();
  WSSSocket(const linear::shared_ptr<linear::SocketImpl>& socket);
  WSSSocket(const linear::shared_ptr<linear::WSSSocketImpl>& wss_socket);
  virtual ~WSSSocket();
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
  /**
   * verify peer certificate
   * @return linaer::Error object
   */
  linear::Error GetVerifyResult() const;
  /**
   * peer certificate exists or not
   * @return false in the case of not receiving peer certificate
   */
  bool PresentPeerCertificate() const;
  /**
   * get peer certificate
   * @return linear::X509Certificate object
   */
  linear::X509Certificate GetPeerCertificate() const;
};

}  // namespace linear

#endif  // LINEAR_WSS_SOCKET_H_
