/**
 * @file tcp_socket.h
 * TCPSocket class definition
 */

#ifndef LINEAR_TCP_SOCKET_H_
#define LINEAR_TCP_SOCKET_H_

#include "linear/socket.h"

namespace linear {

class TCPSocketImpl;

/**
 * @class TCPSocket tcp_socket.h "linear/tcp_socket.h"
 * TCPSocket class that extends Socket class
 */
class LINEAR_EXTERN TCPSocket : public Socket {
 public:
  /// @cond hidden
  TCPSocket();
  TCPSocket(const linear::shared_ptr<linear::SocketImpl>& socket);
  TCPSocket(const linear::shared_ptr<linear::TCPSocketImpl>& tcp_socket);
  ~TCPSocket();
  /// @endcond
};

}  // namespace linear

#endif // LINER_TCP_SOCKET_H_
