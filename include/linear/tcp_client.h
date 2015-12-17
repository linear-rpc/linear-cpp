/**
 * @file tcp_client.h
 * TCPClient class definition
 */

#ifndef LINEAR_TCP_CLIENT_H_
#define LINEAR_TCP_CLIENT_H_

#include "linear/client.h"
#include "linear/handler.h"
#include "linear/tcp_socket.h"

namespace linear {

/**
 * @class TCPClient tcp_client.h "linear/tcp_client.h"
 * TCPClient class that extends Client class
 * @includelineno tcp_client_sample.cpp
 */
class LINEAR_EXTERN TCPClient : public Client {
 public:
  /// @cond hidden
  TCPClient() : Client() {}
  virtual ~TCPClient() {}
  /// @endcond
  /**
   * Constructor
   * @param [in] handler application defined behavior.
   * @param [in] [loop] eventloop(thread) object
   */
  TCPClient(const linear::shared_ptr<linear::Handler>& handler,
            const linear::EventLoop& loop = linear::EventLoop::GetDefault());
  /**
   * Create new TCPSocket Object.
   * @param [in] hostname hostname or IPAddr of a target server.
   * @param [in] port port number of a target server.
   */
  linear::TCPSocket CreateSocket(const std::string& hostname, int port);
};

}  // namespace linear

#endif  // LINEAR_TCP_CLIENT_H_
