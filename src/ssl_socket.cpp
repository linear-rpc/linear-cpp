#include "linear/log.h"
#include "linear/ssl_socket.h"

#include "ssl_socket_impl.h"

using namespace linear::log;

namespace linear {

SSLSocket::SSLSocket() : Socket() {
}

SSLSocket::SSLSocket(const shared_ptr<SocketImpl>& socket) : Socket(socket) {
  if (GetType() != Socket::SSL) {
    LINEAR_LOG(LOG_ERR, "invalid type_cast: type = %d, id = %d", GetType(), GetId());
    throw std::bad_cast();
  }
}

SSLSocket::SSLSocket(const shared_ptr<SSLSocketImpl>& ssl_socket) : Socket(ssl_socket) {
}

SSLSocket::~SSLSocket() {
}

Error SSLSocket::GetVerifyResult() const {
  if (!socket_) {
    return Error(LNR_EBADF);
  }
  return dynamic_pointer_cast<SSLSocketImpl>(socket_)->GetVerifyResult();
}

bool SSLSocket::PresentPeerCertificate() const {
  if (!socket_) {
    return false;
  }
  return dynamic_pointer_cast<SSLSocketImpl>(socket_)->PresentPeerCertificate();
}

X509Certificate SSLSocket::GetPeerCertificate() const {
  if (!socket_) {
    return X509Certificate();
  }
  return dynamic_pointer_cast<SSLSocketImpl>(socket_)->GetPeerCertificate();
}

std::vector<X509Certificate> SSLSocket::GetPeerCertificateChain() const {
  if (!socket_) {
    return std::vector<X509Certificate>();
  }
  return dynamic_pointer_cast<SSLSocketImpl>(socket_)->GetPeerCertificateChain();
}

}  // namespace linear
