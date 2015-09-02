#include "linear/auth_context.h"

#include "auth_context_impl.h"
#include "tv.h"

namespace linear {

// Authorization Header Context
AuthorizationContext::AuthorizationContext() : AuthContext() {
}

AuthorizationContext::AuthorizationContext(const linear::shared_ptr<linear::AuthorizationContextImpl>& impl)
  : AuthContext(), authorization_(impl) {
}

AuthorizationContext::~AuthorizationContext() {
}

AuthorizationContext::AuthorizationContext(const linear::AuthorizationContext& authorization)
  : AuthContext(authorization.type, authorization.username, authorization.realm),
    authorization_(authorization.authorization_) {
}

linear::AuthorizationContext& AuthorizationContext::operator=(const linear::AuthorizationContext& authorization) {
  type = authorization.type;
  username = authorization.username;
  realm = authorization.realm;
  authorization_ = authorization.authorization_;
  return *this;
}

AuthorizationContext::Result AuthorizationContext::Validate(const std::string& password) {
  if (!authorization_) {
    return VALID;
  }
  return authorization_->Validate(password);
}

AuthorizationContext::Result AuthorizationContext::ValidateWithHash(const std::string& hash) {
  if (!authorization_) {
    return VALID;
  }
  return authorization_->ValidateWithHash(hash);
}

// WWW-Authenticate Header Context
AuthenticateContext::AuthenticateContext() : AuthContext() {
}
  
AuthenticateContext::~AuthenticateContext() {
}

AuthenticateContext::AuthenticateContext(const linear::AuthenticateContext& authenticate)
  : AuthContext(authenticate.type, authenticate.username, authenticate.realm),
    password(authenticate.password) {
}

linear::AuthenticateContext& AuthenticateContext::operator=(const linear::AuthenticateContext& authenticate) {
  type = authenticate.type;
  username = authenticate.username;
  realm = authenticate.realm;
  password = authenticate.password;
  return *this;
}

} // namespace linear
