/**
 * @file client.h
 * Super class definition for several concrete client classes
 */

#ifndef LINEAR_CLIENT_H_
#define LINEAR_CLIENT_H_

#include "linear/memory.h"

#include "linear/private/extern.h"

namespace linear {

class ClientImpl;

/**
 * @class Client client.h "linear/client.h"
 * Super class for several concrete client classes
 */
class LINEAR_EXTERN Client {
 public:
  /// @cond hidden
  Client() {}
  virtual ~Client() {}
  /// @endcond

 protected:
  /// @cond hidden
  linear::shared_ptr<linear::ClientImpl> client_;
  /// @endcond
};

} // namespace linear

#endif  // LINEAR_CLIENT_H_
