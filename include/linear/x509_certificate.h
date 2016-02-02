/**
 * @file x509_certificate.h
 * X509Certificate class definition
 **/

#ifndef LINEAR_X509_CERTIFICATE_H_
#define LINEAR_X509_CERTIFICATE_H_

#include <string>

#include "linear/memory.h"
#include "linear/private/extern.h"

#ifdef _WIN32
# include <winsock2.h>
# include <mswsock.h>
# include <ws2tcpip.h>
# include <windows.h>
#endif

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

namespace linear {

/**
 * @class X509Principal x509_certificate.h "linear/x509_certificate.h"
 * X509Principal class
 */
class LINEAR_EXTERN X509Principal {
 public:
  /// @cond hidden
  X509Principal() {}
  explicit X509Principal(X509_NAME* xname);
  ~X509Principal() {}
  /// @endcond
  /**
   * get common name from certificate
   */
  std::string GetCommonName() const;

 public:
  std::string CN;
  std::string DN;
};

/**
 * @class X509Certificate x509_certificate.h "linear/x509_certificate.h"
 * X509Certificate class
 */
class LINEAR_EXTERN X509Certificate {
 public:
  /// @cond hidden
  X509Certificate() {}
  explicit X509Certificate(X509* xcert);
  ~X509Certificate() {}
  /// @endcond
  /**
   * whether this cert is CA cert.
   */
  bool IsCA() const;
  /**
   * get subject from certificate
   */
  X509Principal GetSubject() const;
  /**
   * get issuer from certificate
   */
  X509Principal GetIssuer() const;

  /// @cond hidden
  X509* GetHandle() const;
  /// @endcond

 private:
  class X509CertificateImpl;
  linear::shared_ptr<X509CertificateImpl> pimpl_;
};

}  // namespace linear

#endif  // LINEAR_X509_CERTIFICATE_H_
