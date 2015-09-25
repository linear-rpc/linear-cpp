/**
 * @file socket.h
 * Super class definition for several concrete socket classes
 */

#ifndef LINEAR_SOCKET_H_
#define LINEAR_SOCKET_H_

#include "linear/addrinfo.h"
#include "linear/error.h"
#include "linear/memory.h"

namespace linear {

class Message;
class SocketImpl;

/**
 * @class Socket socket.h "linear/socket.h"
 * Super class for several concrete socket classes
 */
class LINEAR_EXTERN Socket {
 public:
  //! default max message buffer size (64MB)
  static const size_t DEFAULT_MAX_BUFFER_SIZE = (64 * 1024 * 1024);

  //! socket type indicator
  enum Type {
    NIL, //!< NIL
    TCP, //!< TCP
    SSL, //!< SSL
    WS,  //!< WebSocket
    WSS, //!< Secure WebSocket
  };

  //! socket state indicator
  enum State {
    DISCONNECTING, //!< Disconnecting
    DISCONNECTED,  //!< Disconnected
    CONNECTING,    //!< Connecting
    CONNECTED,     //!< Connected
  };

  enum KeepAliveType {
    KEEPALIVE_TCP, //!< use TCP_KEEPALIVE
    KEEPALIVE_WS,  //!< use WS_KEEPALIVE
  };

 public:
  /// @cond hidden
  Socket();
  Socket(const linear::shared_ptr<linear::SocketImpl>& socket);
  virtual ~Socket();
  /// @endcond

  Socket(const linear::Socket& socket);
  linear::Socket& operator=(const linear::Socket& socket);

  bool operator==(const linear::Socket& socket) const;
  bool operator!=(const linear::Socket& socket) const;
  bool operator<(const linear::Socket& socket) const;
  bool operator>(const linear::Socket& socket) const;
  bool operator<=(const linear::Socket& socket) const;
  bool operator>=(const linear::Socket& socket) const;

  /**
   * downcast method to get concrete Socket
   * @see linear::TCPSocket, linear::SSLSocket, linear::WSSocket, linear::WSSSocket
   */
  template <typename SocketType>
  inline SocketType as() const {
    return SocketType(socket_);
  }
  /**
   * set max message buffer size.
   * @param [in] max_limit max limit of message size (byte)
   * @return linear::Error object
   * @see linaer::Socket::DEFAULT_MAX_BUFFER_SIZE
   */
  virtual linear::Error SetMaxBufferSize(size_t max_limit) const;
  /**
   * connect to target.
   * @param [in] timeout connect timeout(msec)\n
   * 0 as default. means system default timeout sec.
   * @return linear::Error object
   */
  virtual linear::Error Connect(unsigned int timeout = 0) const;
  /**
   * disconnect from target.
   * @return linear::Error object
   */
  virtual linear::Error Disconnect() const;
  /**
   * @fn linear::Error KeepAlive(int interval, int retry)
   * Interface to set SO_KEEPALIVE
   * @param [in] interval time(second) to send tcp-keepalive
   * @param [in] retry retry counter(3 times as default)
   * @param [in] type keepalive type (TCP_KEEPALIVE as default)
   * @return linear::Error object
   * @note
   * TCP_KEEPALIVE is enabled on Windows, linux 2.4- and iOS 7.0-.
   * refer man 2 socket and man 7 tcp
   */
  virtual linear::Error KeepAlive(unsigned int interval = 1, unsigned int retry = 3, linear::Socket::KeepAliveType type = Socket::KEEPALIVE_TCP) const;
  /**
   * @fn linear::Error BindToDevice(const std::string& ifname)
   * Interface to set SO_BINDTODEVICE
   * @param [in] ifname interface name
   * @return linear::Error object
   * @note
   */
  virtual linear::Error BindToDevice(const std::string& ifname) const;
  /**
   * @fn linear::Error SetSockOpt(int level, int optname, const void* optval, size_t optlen);
   * Interface to setsockopt
   * @param [in] level protocol number
   * @param [in] optname option name
   * @param [in] optval option value
   * @param [in] optlen option value length
   * @return linear::Error object
   * @note
   */
  virtual linear::Error SetSockOpt(int level, int optname, const void* optval, size_t optlen) const;
  /**
   * get socket id.
   * @return integer number or -1 if invalid
   */
  virtual int GetId() const;
  /**
   * get socket type.
   * @return linear::Socket::Type
   */
  virtual linear::Socket::Type GetType() const;
  /**
   * get socket state.
   * @return linear::Socket::State
   */
  virtual linear::Socket::State GetState() const;
  /**
   * get self socket information.
   * @return linear::Addrinfo
   */
  virtual const linear::Addrinfo& GetSelfInfo() const;
  /**
   * get peer socket information.
   * @return linear::Addrinfo
   */
  virtual const linear::Addrinfo& GetPeerInfo() const;

  // @cond hidden
  virtual linear::Error Send(const linear::Message& message, int timeout = 30000) const;
  // @endcond

 protected:
  // @cond hidden
  linear::shared_ptr<SocketImpl> socket_;
  // @endcond
};

}  // namespace linear

#endif  // LINEAR_SOCKET_H_
