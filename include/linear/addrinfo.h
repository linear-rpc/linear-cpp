/**
 * @file addrinfo.h
 * Address Information class definition
 */

#ifndef LINEAR_ADDRINFO_H_
#define LINEAR_ADDRINFO_H_

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
#endif

#include <string>

#include "linear/private/extern.h"

namespace linear {

/**
 * @class Addrinfo addrinfo.h "linear/addrinfo.h"
 * Address Informations
 */
struct LINEAR_EXTERN Addrinfo {

  //! address family
  enum Protocol {
    UNKNOWN = -1,
    IPv4,
    IPv6
  };

  /// @cond hidden
  Addrinfo();
  Addrinfo(const struct sockaddr* sa);
  Addrinfo(const std::string& a, int p);
  ~Addrinfo();
  /// @endcond

  std::string                   addr;  //!< ip address of node
  int                           port;  //!< port number of node
  linear::Addrinfo::Protocol    proto; //!< address family indicator
};

}  // namespace linear

#endif  // LINEAR_ADDRINFO_H_
