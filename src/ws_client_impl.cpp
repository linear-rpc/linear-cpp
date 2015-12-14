#include "ws_client_impl.h"
#include "ws_socket_impl.h"

using namespace linear::log;

namespace linear {

WSClientImpl::WSClientImpl(const Handler& handler, const WSRequestContext& request_context)
  : ClientImpl(handler), request_context_(request_context) {
}

WSClientImpl::~WSClientImpl() {
}

void WSClientImpl::SetWSRequestContext(const WSRequestContext& request_context) {
  request_context_ = request_context;
}

WSSocket WSClientImpl::CreateSocket(const std::string& hostname, int port) {
  return this->CreateSocket(hostname, port, request_context_);
}

WSSocket WSClientImpl::CreateSocket(const std::string& hostname, int port, const WSRequestContext& request_context) {
  try {
    return WSSocket(shared_ptr<WSSocketImpl>(new WSSocketImpl(hostname, port, request_context, *this)));
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

}  // namespace linear
