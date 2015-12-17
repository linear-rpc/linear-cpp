#include "linear/log.h"
#include "linear/wss_socket.h"

#include "wss_socket_impl.h"

using namespace linear::log;

namespace linear {

WSSSocket::WSSSocket() : Socket() {
}

WSSSocket::WSSSocket(const shared_ptr<SocketImpl>& socket) : Socket(socket) {
  if (GetType() != Socket::WSS) {
    LINEAR_LOG(LOG_ERR, "invalid type_cast: type = %d, id = %d", GetType(), GetId());
    throw std::bad_cast();
  }
}

WSSSocket::WSSSocket(const shared_ptr<WSSSocketImpl>& wss_socket) : Socket(wss_socket) {
}

WSSSocket::~WSSSocket() {
}

const WSRequestContext& WSSSocket::GetWSRequestContext() const {
  if (!socket_) {
    static WSRequestContext context;
    return context;
  }
  return dynamic_pointer_cast<WSSSocketImpl>(socket_)->GetWSRequestContext();
}

void WSSSocket::SetWSRequestContext(const WSRequestContext& request_context) const {
  if (!socket_) {
    return;
  }
  return dynamic_pointer_cast<WSSSocketImpl>(socket_)->SetWSRequestContext(request_context);
}

const WSResponseContext& WSSSocket::GetWSResponseContext() const {
  if (!socket_) {
    static WSResponseContext context;
    return context;
  }
  return dynamic_pointer_cast<WSSSocketImpl>(socket_)->GetWSResponseContext();
}

void WSSSocket::SetWSResponseContext(const WSResponseContext& response_context) const {
  if (!socket_) {
    return;
  }
  return dynamic_pointer_cast<WSSSocketImpl>(socket_)->SetWSResponseContext(response_context);
}

Error WSSSocket::GetVerifyResult() const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  return dynamic_pointer_cast<WSSSocketImpl>(socket_)->GetVerifyResult();
}

bool WSSSocket::PresentPeerCertificate() const {
  if (!socket_) {
    return false;
  }
  return dynamic_pointer_cast<WSSSocketImpl>(socket_)->PresentPeerCertificate();
}

X509Certificate WSSSocket::GetPeerCertificate() const {
  if (!socket_) {
    return X509Certificate();
  }
  return dynamic_pointer_cast<WSSSocketImpl>(socket_)->GetPeerCertificate();
}

}  // namespace linear
