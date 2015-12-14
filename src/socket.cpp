#include "linear/log.h"

#include "socket_impl.h"

using namespace linear::log;

namespace linear {

static std::string GetTypeString(Socket::Type type) {
  std::string proto("NIL");
  switch(type) {
  case Socket::TCP:
    proto = "TCP";
    break;
  case Socket::SSL:
    proto = "SSL";
    break;
  case Socket::WS:
    proto = "WS";
    break;
  case Socket::WSS:
    proto = "WSS";
    break;
  case Socket::NIL:
  default:
    break;
  }
  return proto;  
}

Socket::Socket() : socket_() {
}

Socket::Socket(const shared_ptr<SocketImpl>& socket) : socket_(socket) {
}

Socket::~Socket() {
}

Socket::Socket(const Socket& socket) : socket_(socket.socket_) {
}

Socket& Socket::operator=(const Socket& socket) {
  socket_ = socket.socket_;
  return *this;
}

bool Socket::operator==(const Socket& socket) const {
  return (GetId() == socket.GetId());
}

bool Socket::operator!=(const Socket& socket) const {
  return (GetId() != socket.GetId());
}

bool Socket::operator<(const Socket& socket) const {
  return (GetId() < socket.GetId());
}

bool Socket::operator>(const Socket& socket) const {
  return (GetId() > socket.GetId());
}

bool Socket::operator<=(const Socket& socket) const {
  return (GetId() <= socket.GetId());
}

bool Socket::operator>=(const Socket& socket) const {
  return (GetId() >= socket.GetId());
}

Error Socket::SetMaxBufferSize(size_t limit) const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  socket_->SetMaxBufferSize(limit);
  return Error(LNR_OK);
}

Error Socket::Connect(unsigned int timeout) const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  Error e(LNR_ENOMEM);
  try {
    EventLoopImpl::SocketEvent* ev = new EventLoopImpl::SocketEvent(socket_);
    e = socket_->Connect(timeout, ev);
    if (e != Error(LNR_OK)) {
      delete ev;
    }
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
  }
  return e;
}

Error Socket::Disconnect() const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  const linear::Addrinfo& self = GetSelfInfo();
  const linear::Addrinfo& peer = GetPeerInfo();
  LINEAR_LOG(LOG_DEBUG, "try to disconnect(id = %d): %s:%d x-- %s --- %s:%d",
             GetId(),
             self.addr.c_str(), self.port, GetTypeString(GetType()).c_str(),
             peer.addr.c_str(), peer.port);
  return socket_->Disconnect();
}

Error Socket::KeepAlive(unsigned int interval, unsigned int retry, KeepAliveType type) const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  return socket_->KeepAlive(interval, retry, type);
}

Error Socket::BindToDevice(const std::string& ifname) const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  return socket_->BindToDevice(ifname);
}

Error Socket::SetSockOpt(int level, int optname, const void* optval, size_t optlen) const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  return socket_->SetSockOpt(level, optname, optval, optlen);
}

int Socket::GetId() const {
  if (!socket_) {
    return -1;
  }
  return socket_->GetId();
}

Socket::Type Socket::GetType() const {
  if (!socket_) {
    return Socket::NIL;
  }
  return socket_->GetType();
}

Socket::State Socket::GetState() const {
  if (!socket_) {
    return DISCONNECTED;
  }
  return socket_->GetState();
}

const Addrinfo& Socket::GetSelfInfo() const {
  static Addrinfo info;
  if (!socket_) {
    return info;
  }
  return socket_->GetSelfInfo();
}

const Addrinfo& Socket::GetPeerInfo() const {
  static Addrinfo info;
  if (!socket_) {
    return info;
  }
  return socket_->GetPeerInfo();
}

Error Socket::Send(const Message& message, int timeout) const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  return socket_->Send(message, timeout);
}

} // namespace linear
