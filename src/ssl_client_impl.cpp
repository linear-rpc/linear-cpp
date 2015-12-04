#include "linear/ssl_socket.h"

#include "ssl_client_impl.h"
#include "ssl_socket_impl.h"

using namespace linear::log;

namespace linear {

SSLClientImpl::SSLClientImpl(const Handler& handler, const SSLContext& context)
  : ClientImpl(handler, true), context_(context) {
}

SSLClientImpl::~SSLClientImpl() {
}

void SSLClientImpl::SetContext(const linear::SSLContext& context) {
  context_ = context;
}

SSLSocket SSLClientImpl::CreateSocket(const std::string& hostname, int port) {
  return this->CreateSocket(hostname, port, context_);
}

SSLSocket SSLClientImpl::CreateSocket(const std::string& hostname, int port,
                                      const linear::SSLContext& context) {
  try {
    return SSLSocket(shared_ptr<SSLSocketImpl>(new SSLSocketImpl(hostname, port, context, *this)));
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

}  // namespace linear
