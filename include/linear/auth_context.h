/**
 * @file auth_context.h
 * Authentication and Authorization Information class definition
 */

#ifndef LINEAR_AUTH_CONTEXT_H_
#define LINEAR_AUTH_CONTEXT_H_

#include <string>
#include <map>

#include "linear/memory.h"
#include "linear/private/extern.h"

namespace linear {

class AuthenticateContextImpl;
class AuthorizationContextImpl;

/**
 * @class AuthContext auth_context.h "linear/auth_context.h"
 * Super class for Authentication and Authorization
 */
class LINEAR_EXTERN AuthContext {
 public:
  //! auth type indicator
  enum Type {
    UNUSED,  //!< UNUSED
    BASIC,   //!< Basic Authentication
    DIGEST,  //!< Digest Authentication
    UNKNOWN, //!< Unknown Authentication
  };

  /// @cond hidden
  AuthContext(linear::AuthContext::Type t = linear::AuthContext::UNUSED,
              const std::string& u = "", const std::string& r = "")
    : type(t), username(u), realm(r) {}
  virtual ~AuthContext() {}
  /// @endcond

  linear::AuthContext::Type type; //!< authentication/authorization type
  std::string username;           //!< user name
  std::string realm;              //!< realm
};

/**
 * @class AuthenticateContext auth_context.h "linear/auth_context.h"
 * Authentication Information
 */
class LINEAR_EXTERN AuthenticateContext : public AuthContext {
 public:
  /// @cond hidden
  AuthenticateContext();
  ~AuthenticateContext();
  AuthenticateContext(const linear::AuthenticateContext& authenticate);
  linear::AuthenticateContext& operator=(const linear::AuthenticateContext& authenticate);
  /// @endcond

  std::string password; //!< password
};

/**
 * @class AuthorizationContext auth_context.h "linear/auth_context.h"
 * Authorization Information
 */
class LINEAR_EXTERN AuthorizationContext : public AuthContext {
 public:
  //! validate result indicator
  enum Result {
    INVALID   = -1, //!< invalid password
    VALID     =  0, //!< valid password
    AMBIGUOUS =  1, //!< valid password but other informations are not valid
  };

  /// @cond hidden
  AuthorizationContext();
  AuthorizationContext(const linear::shared_ptr<linear::AuthorizationContextImpl>& impl);
  ~AuthorizationContext();
  AuthorizationContext(const linear::AuthorizationContext& authorization);
  linear::AuthorizationContext& operator=(const linear::AuthorizationContext& authorization);
  /// @endcond

  /**
   * Validate authorization response by password
   * @param [in] password password for username and realm
   * @return linear::AuthorizationContext::Result
   **/
  Result Validate(const std::string& password);
  /**
   * Validate authorization response by hash
   * @param [in] hash hashed password := MD5(username:realm:password)
   * @return linear::AuthorizationContext::Result
   **/
  Result ValidateWithHash(const std::string& hash);

 private:
  linear::shared_ptr<AuthorizationContextImpl> authorization_;
};

}  // namespace linear

#endif  // LINEAR_AUTH_CONTEXT_H_
