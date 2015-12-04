/**
 * @file ssl_socket.h
 * SSLSocket class definition
 */

#ifndef LINEAR_SSL_SOCKET_H_
#define LINEAR_SSL_SOCKET_H_

#include "linear/socket.h"
#include "linear/x509_certificate.h"

namespace linear {

class SSLSocketImpl;

/**
 * @class SSLSocket ssl_socket.h "linear/ssl_socket.h"
 * SSLSocket class that extends Socket class
 */
class LINEAR_EXTERN SSLSocket : public Socket {
 public:
  /// @cond hidden
  SSLSocket();
  explicit SSLSocket(const linear::shared_ptr<linear::SocketImpl>& socket);
  explicit SSLSocket(const linear::shared_ptr<linear::SSLSocketImpl>& ssl_socket);
  virtual ~SSLSocket();
  /// @endcond

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

#endif  // LINEAR_SSL_SOCKET_H_
