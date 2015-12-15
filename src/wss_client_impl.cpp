#include "wss_client_impl.h"
#include "wss_socket_impl.h"

using namespace linear::log;

namespace linear {

WSSClientImpl::WSSClientImpl(const Handler& handler,
                             const WSRequestContext& request_context,
                             const linear::SSLContext& ssl_context,
                             const linear::EventLoop& loop)
  : ClientImpl(handler, loop, true),
    request_context_(request_context), ssl_context_(ssl_context) {
}

WSSClientImpl::~WSSClientImpl() {
}

void WSSClientImpl::SetWSRequestContext(const WSRequestContext& request_context) {
  request_context_ = request_context;
}

void WSSClientImpl::SetSSLContext(const SSLContext& ssl_context) {
  ssl_context_ = ssl_context;
}

WSSSocket WSSClientImpl::CreateSocket(const std::string& hostname, int port) {
  return this->CreateSocket(hostname, port, request_context_, ssl_context_);
}

WSSSocket WSSClientImpl::CreateSocket(const std::string& hostname, int port,
                                      const WSRequestContext& request_context) {
  return this->CreateSocket(hostname, port, request_context, ssl_context_);
}

WSSSocket WSSClientImpl::CreateSocket(const std::string& hostname, int port,
                                      const SSLContext& ssl_context) {
  return this->CreateSocket(hostname, port, request_context_, ssl_context);
}

WSSSocket WSSClientImpl::CreateSocket(const std::string& hostname, int port,
                                      const WSRequestContext& request_context,
                                      const SSLContext& ssl_context) {
  try {
    return WSSSocket(shared_ptr<WSSSocketImpl>(new WSSSocketImpl(hostname, port, request_context, ssl_context, loop_, *this)));
  } catch (...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

}  // namespace linear
