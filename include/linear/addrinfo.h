/**
 * @file addrinfo.h
 * Address Information class definition
 */

#ifndef LINEAR_ADDRINFO_H_
#define LINEAR_ADDRINFO_H_

#ifdef _WIN32
# include <winsock2.h>
#else
# include <netinet/in.h>
#endif

#include <string>

#include "linear/private/extern.h"

namespace linear {

/**
 * @class Addrinfo addrinfo.h "linear/addrinfo.h"
 * Address Informations
 */
struct LINEAR_EXTERN Addrinfo {
  /// @cond hidden
  Addrinfo();
  Addrinfo(const struct sockaddr* sa);
  Addrinfo(const std::string& a, int p);
  ~Addrinfo();
  /// @endcond

  std::string addr; //!< ip address of node
  int         port; //!< port number of node
};

}  // namespace linear

#endif  // LINEAR_ADDRINFO_H_
