#ifndef LINEAR_AUTH_CONTEXT_IMPL_H_
#define LINEAR_AUTH_CONTEXT_IMPL_H_

#include "linear/auth_context.h"

namespace linear {

class AuthContextImpl {
 public:
  AuthContextImpl() : type(linear::AuthContext::UNUSED) {}
  AuthContextImpl(linear::AuthContext::Type t) : type(t) {}
  virtual ~AuthContextImpl() {}

  linear::AuthContext::Type type;
};

// linear-cpp can handle below
// digest-response = 1#(username|realm|nonce|digest-uri|response|
//                      |[cnonce]|[message-qop]|[nonce-count])
// message-qop     = "qop=auth"
//
// RFC2617 3.2.2 - The Authorization Request Header
// digest-response = 1#(username|realm|nonce|digest-uri|response|
//                      [algorithm]|[cnonce]|[opaque]|[message-qop]|[nonce-count]|[auth-param])
//
class AuthorizationContextImpl : public AuthContextImpl {
 public:
  AuthorizationContextImpl(const std::string& v);
  ~AuthorizationContextImpl();

  linear::AuthorizationContext::Result Validate(const std::string& password);
  linear::AuthorizationContext::Result ValidateWithHash(const std::string& hash);

  std::string username;
  std::string nonce;
  bool valid_nonce;
  std::string realm;

 private:
  std::string uri;
  std::string qop;
  std::string nc;
  std::string cnonce;
  std::string response;
};

class AuthenticateContextImpl : public AuthContextImpl {
 public:
  AuthenticateContextImpl() : AuthContextImpl(), nc(0) {}
  AuthenticateContextImpl(const std::string& v);
  ~AuthenticateContextImpl();

  std::string CreateAuthorizationHeader(const std::string& uri, const std::string& username, const std::string& password);

  int nc;

 private:
  std::string realm;
  std::string nonce;
  std::string algorithm;
  std::string qop;
};

}  // namespace linear

#endif  // LINEAR_AUTH_CONTEXT_IMPL_H_
