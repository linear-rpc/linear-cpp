/**
 * @file error.h
 * Error class definition
 **/

#ifndef LINEAR_ERROR_H_
#define LINEAR_ERROR_H_

#include <string>

#include "linear/private/extern.h"

namespace linear {

/**
 * @enum linear::ErrorCode
 * list of ErrorCodes
 * @var linear::LNR_OK
 * success
 * @var linear::LNR_EACCES
 * permission denied
 * @var linear::LNR_EADDRINUSE
 * address already in use
 * @var linear::LNR_EADDRNOTAVAIL
 * address not available
 * @var linear::LNR_EAFNOSUPPORT
 * address family not supported
 * @var linear::LNR_EAGAIN
 * resource temporarily unavailable
 * @var linear::LNR_EAI_ADDRFAMILY
 * address family not supported
 * @var linear::LNR_EAI_AGAIN
 * temporary failure
 * @var linear::LNR_EAI_BADFLAGS
 * bad ai_flags value
 * @var linear::LNR_EAI_CANCELED
 * request canceled
 * @var linear::LNR_EAI_FAIL
 * permanent failure
 * @var linear::LNR_EAI_FAMILY
 * ai_family not supported
 * @var linear::LNR_EAI_MEMORY
 * out of memory
 * @var linear::LNR_EAI_NODATA
 * no address
 * @var linear::LNR_EAI_NONAME
 * unknown node or service
 * @var linear::LNR_EAI_SERVICE
 * service not available for socket type
 * @var linear::LNR_EAI_SOCKTYPE
 * socket type not supported
 * @var linear::LNR_EALREADY
 * connection already in progress
 * @var linear::LNR_EBADF
 * bad file descriptor
 * @var linear::LNR_EBUSY
 * resource busy or locked
 * @var linear::LNR_ECANCELED
 * operation canceled
 * @var linear::LNR_ECHARSET
 * invalid Unicode character
 * @var linear::LNR_ECONNABORTED
 * software caused connection abort
 * @var linear::LNR_ECONNREFUSED
 * connection refused
 * @var linear::LNR_ECONNRESET
 * connection reset by peer
 * @var linear::LNR_EDESTADDRREQ
 * destination address required
 * @var linear::LNR_EEXIST
 * file already exists
 * @var linear::LNR_EFAULT
 * bad address in system call argument
 * @var linear::LNR_EHOSTUNREACH
 * host is unreachable
 * @var linear::LNR_EINTR
 * interrupted system call
 * @var linear::LNR_EINVAL
 * invalid argument
 * @var linear::LNR_EIO
 * i/o error
 * @var linear::LNR_EISCONN
 * socket is already connected
 * @var linear::LNR_EISDIR
 * illegal operation on a directory
 * @var linear::LNR_ELOOP
 * too many symbolic links encountered
 * @var linear::LNR_EMFILE
 * too many open files
 * @var linear::LNR_EMSGSIZE
 * message too long
 * @var linear::LNR_ENAMETOOLONG
 * name too long
 * @var linear::LNR_ENETDOWN
 * network is down
 * @var linear::LNR_ENETUNREACH
 * network is unreachable
 * @var linear::LNR_ENFILE
 * file table overflow
 * @var linear::LNR_ENOBUFS
 * no buffer space available
 * @var linear::LNR_ENODEV
 * no such device
 * @var linear::LNR_ENOENT
 * no such file or directory
 * @var linear::LNR_ENOMEM
 * not enough memory
 * @var linear::LNR_ENONET
 * machine is not on the network
 * @var linear::LNR_ENOSPC
 * no space left on device
 * @var linear::LNR_ENOSYS
 * function not implemented
 * @var linear::LNR_ENOTCONN
 * socket is not connected
 * @var linear::LNR_ENOTDIR
 * not a directory
 * @var linear::LNR_ENOTEMPTY
 * directory not empty
 * @var linear::LNR_ENOTSOCK
 * socket operation on non-socket
 * @var linear::LNR_ENOTSUP
 * operation not supported on socket
 * @var linear::LNR_EPERM
 * operation not permitted
 * @var linear::LNR_EPIPE
 * broken pipe
 * @var linear::LNR_EPROTO
 * protocol error
 * @var linear::LNR_EPROTONOSUPPORT
 * protocol not supported
 * @var linear::LNR_EPROTOTYPE
 * protocol wrong type for socket
 * @var linear::LNR_EROFS
 * read-only file system
 * @var linear::LNR_ESHUTDOWN
 * cannot send after transport endpoint shutdown
 * @var linear::LNR_ESPIPE
 * invalid seek
 * @var linear::LNR_ESRCH
 * no such process
 * @var linear::LNR_ETIMEDOUT
 * connection timed out
 * @var linear::LNR_EXDEV
 * cross-device link not permitted
 * @var linear::LNR_UNKNOWN
 * unknown error
 * @var linear::LNR_EOF
 * end of file
 * @var linear::LNR_EX509
 * X509 certificate error
 * @var linear::LNR_ESSL
 * SSL protocol error
 * @var linear::LNR_EWS
 * WebSocket protocol error
 */
#define LNR_ERRNO_MAP(XX)                                               \
  XX(EACCES, "permission denied")                                       \
  XX(EADDRINUSE, "address already in use")                              \
  XX(EADDRNOTAVAIL, "address not available")                            \
  XX(EAFNOSUPPORT, "address family not supported")                      \
  XX(EAGAIN, "resource temporarily unavailable")                        \
  XX(EAI_ADDRFAMILY, "address family not supported")                    \
  XX(EAI_AGAIN, "temporary failure")                                    \
  XX(EAI_BADFLAGS, "bad ai_flags value")                                \
  XX(EAI_CANCELED, "request canceled")                                  \
  XX(EAI_FAIL, "permanent failure")                                     \
  XX(EAI_FAMILY, "ai_family not supported")                             \
  XX(EAI_MEMORY, "out of memory")                                       \
  XX(EAI_NODATA, "no address")                                          \
  XX(EAI_NONAME, "unknown node or service")                             \
  XX(EAI_SERVICE, "service not available for socket type")              \
  XX(EAI_SOCKTYPE, "socket type not supported")                         \
  XX(EALREADY, "connection already in progress")                        \
  XX(EBADF, "bad file descriptor")                                      \
  XX(EBUSY, "resource busy or locked")                                  \
  XX(ECANCELED, "operation canceled")                                   \
  XX(ECHARSET, "invalid Unicode character")                             \
  XX(ECONNABORTED, "software caused connection abort")                  \
  XX(ECONNREFUSED, "connection refused")                                \
  XX(ECONNRESET, "connection reset by peer")                            \
  XX(EDESTADDRREQ, "destination address required")                      \
  XX(EEXIST, "file already exists")                                     \
  XX(EFAULT, "bad address in system call argument")                     \
  XX(EHOSTUNREACH, "host is unreachable")                               \
  XX(EINTR, "interrupted system call")                                  \
  XX(EINVAL, "invalid argument")                                        \
  XX(EIO, "i/o error")                                                  \
  XX(EISCONN, "socket is already connected")                            \
  XX(EISDIR, "illegal operation on a directory")                        \
  XX(ELOOP, "too many symbolic links encountered")                      \
  XX(EMFILE, "too many open files")                                     \
  XX(EMSGSIZE, "message too long")                                      \
  XX(ENAMETOOLONG, "name too long")                                     \
  XX(ENETDOWN, "network is down")                                       \
  XX(ENETUNREACH, "network is unreachable")                             \
  XX(ENFILE, "file table overflow")                                     \
  XX(ENOBUFS, "no buffer space available")                              \
  XX(ENODEV, "no such device")                                          \
  XX(ENOENT, "no such file or directory")                               \
  XX(ENOMEM, "not enough memory")                                       \
  XX(ENONET, "machine is not on the network")                           \
  XX(ENOSPC, "no space left on device")                                 \
  XX(ENOSYS, "function not implemented")                                \
  XX(ENOTCONN, "socket is not connected")                               \
  XX(ENOTDIR, "not a directory")                                        \
  XX(ENOTEMPTY, "directory not empty")                                  \
  XX(ENOTSOCK, "socket operation on non-socket")                        \
  XX(ENOTSUP, "operation not supported on socket")                      \
  XX(EPERM, "operation not permitted")                                  \
  XX(EPIPE, "broken pipe")                                              \
  XX(EPROTO, "protocol error")                                          \
  XX(EPROTONOSUPPORT, "protocol not supported")                         \
  XX(EPROTOTYPE, "protocol wrong type for socket")                      \
  XX(EROFS, "read-only file system")                                    \
  XX(ESHUTDOWN, "cannot send after transport endpoint shutdown")        \
  XX(ESPIPE, "invalid seek")                                            \
  XX(ESRCH, "no such process")                                          \
  XX(ETIMEDOUT, "timed out")                                            \
  XX(EXDEV, "cross-device link not permitted")                          \
  XX(UNKNOWN, "unknown error")                                          \
  XX(EOF, "end of file")                                                \
  XX(EX509, "X509 Certificate error")                                   \
  XX(ESSL, "SSL error")                                                 \
  XX(EWS, "WebSocket error")                                            \

#define LNR_ERRNO_GEN(name, s) LNR_##name,
enum ErrorCode {
  LNR_OK = 0,
  LNR_ERRNO_MAP(LNR_ERRNO_GEN)
  /// @cond hidden
  LNR_MAX_ERRORS
  /// @endcond
};
#undef LNR_ERRNO_GEN

/**
 * @class Error error.h "linear/error.h"
 * Error informations
 * @see linear::ErrorCode
 */
class LINEAR_EXTERN Error {
 public:
  /// @cond hidden
  Error();
  Error(linear::ErrorCode code, unsigned long detail_code = 0);
  Error(int code, unsigned long detail_code = 0);
  ~Error();
  /// @endcond

  /**
   * enable to use == operator
   **/
  bool operator==(const linear::Error& err) const;
  /**
   * enable to use != operator
   **/
  bool operator!=(const linear::Error& err) const;
  /**
   * get error code
   * @return error code
   */
  const linear::ErrorCode& Code() const;
  /**
   * get detail error code
   * @return detail code
   */
  unsigned long DetailCode() const;
  /**
   * get error message
   * @return error message
   */
  std::string Message() const;

 private:
  ErrorCode code_;
  unsigned long detail_code_;
};

} // namespace linear

#endif  // LINEAR_ERROR_H_
