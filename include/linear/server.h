/**
 * @file server.h
 * Super class definition for several concrete server classes
 */

#ifndef LINEAR_SERVER_H_
#define LINEAR_SERVER_H_

#include "linear/error.h"
#include "linear/memory.h"

namespace linear {

class ServerImpl;

/**
 * @class Server server.h "linear/server.h"
 * Super class for several concrete server classes
 */
class LINEAR_EXTERN Server {
 public:
  /// @cond hidden
  Server() {}
  virtual ~Server() {}
  /// @endcond

  /**
   * Set number of clients limit
   * @param [in] max_clients number of clients
   * default == -1: no limit
   * @return linear::Error object
   */
  virtual linear::Error SetMaxClients(int max_clients);
  /**
   * Starts a server with specified parameters.
   * @param [in] hostname IPAddr or FQDN of host
   * @param [in] port portnumber
   * @return linear::Error object
   */
  virtual linear::Error Start(const std::string& hostname, int port);
  /**
   * Stops a server.
   * @return linear::Error object
   */
  virtual linear::Error Stop();

 protected:
  /// @cond hidden
  linear::shared_ptr<linear::ServerImpl> server_;
  /// @endcond
};

}  // namespace linear

#endif  // LINEAR_SERVER_H_
