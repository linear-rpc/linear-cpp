#include "linear/log.h"
#include "linear/ws_socket.h"

#include "ws_socket_impl.h"

using namespace linear::log;

namespace linear {

WSSocket::WSSocket() : Socket() {
}

WSSocket::WSSocket(const shared_ptr<SocketImpl>& socket) : Socket(socket) {
  if (GetType() != Socket::WS) {
    LINEAR_LOG(LOG_ERR, "invalid type_cast: type = %d, id = %d", GetType(), GetId());
    throw std::bad_cast();
  }
}

WSSocket::WSSocket(const shared_ptr<WSSocketImpl>& ws_socket) : Socket(ws_socket) {
}

WSSocket::~WSSocket() {
}

const linear::WSRequestContext& WSSocket::GetWSRequestContext() const {
  if (!socket_) {
    static WSRequestContext context;
    return context;
  }
  return dynamic_pointer_cast<WSSocketImpl>(socket_)->GetWSRequestContext();
}

void WSSocket::SetWSRequestContext(const WSRequestContext& request_context) const {
  if (!socket_) {
    return;
  }
  return dynamic_pointer_cast<WSSocketImpl>(socket_)->SetWSRequestContext(request_context);
}

const linear::WSResponseContext& WSSocket::GetWSResponseContext() const {
  if (!socket_) {
    static WSResponseContext context;
    return context;
  }
  return dynamic_pointer_cast<WSSocketImpl>(socket_)->GetWSResponseContext();
}

void WSSocket::SetWSResponseContext(const WSResponseContext& response_context) const {
  if (!socket_) {
    return;
  }
  return dynamic_pointer_cast<WSSocketImpl>(socket_)->SetWSResponseContext(response_context);
}

}  // namespace linear
