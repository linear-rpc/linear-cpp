/**
 * @file ssl_context.h
 * SSLContext class definition
 */

#ifndef LINEAR_SSL_CONTEXT_H_
#define LINEAR_SSL_CONTEXT_H_

#include "linear/memory.h"
#include "linear/x509_certificate.h"

namespace linear {

/**
 * @class SSLContext ssl_context.h "linear/ssl_context.h"
 * SSLContext class 
 **/
class LINEAR_EXTERN SSLContext {
 public:
  //! SSL Method indicator
  enum Method {
    SSLv23,
    SSLv23_client,
    SSLv23_server,
    TLSv1_1,
    TLSv1_1_client,
    TLSv1_1_server
  };

  //! SSL Verify Mode indicator
  enum VerifyMode {
    VERIFY_NONE,
    VERIFY_PEER,
    VERIFY_FAIL_IF_NO_PEER_CERT
  };

  /// @cond hidden
  SSLContext();
  SSLContext(const SSLContext& obj);
  SSLContext& operator=(const SSLContext& obj);
  ~SSLContext();
  /// @endcond

  /**
   * Constructor
   * @param [in] method SSLMethod
   * @see linear::SSLContext::Method
   */
  explicit SSLContext(const Method& method);

  bool SetCertificate(const std::string& certfile);
  bool SetPrivateKey(const std::string& pkeyfile); // TODO: need?
  bool SetPrivateKey(const std::string& pkeyfile, const std::string& passphrase);
  bool SetCiphers(const std::string& ciphers);
  bool SetCAFile(const std::string& cafile);
  bool SetCAPath(const std::string& capath);
  void SetVerifyMode(const VerifyMode& mode, int (*verify_callback)(int, X509_STORE_CTX*) = NULL);

  /// @cond hidden
  SSL_CTX* GetHandle() const;
  /// @endcond

 private:
  class SSLContextImpl;
  linear::shared_ptr<SSLContextImpl> pimpl_;
};

}  // namespace linear

#endif  // LINEAR_SSL_CONTEXT_H_
