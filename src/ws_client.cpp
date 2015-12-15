#include "linear/ws_client.h"

#include "ws_client_impl.h"

using namespace linear::log;

namespace linear {

WSClient::WSClient(const Handler& handler,
                   const WSRequestContext& request_context,
                   const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<WSClientImpl>(new WSClientImpl(handler, request_context, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSClient::WSClient(const Handler& handler, const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<WSClientImpl>(new WSClientImpl(handler, linear::WSRequestContext(), loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

void WSClient::SetWSRequestContext(const WSRequestContext& request_context) {
  if (client_) {
    static_pointer_cast<WSClientImpl>(client_)->SetWSRequestContext(request_context);
  }
}

WSSocket WSClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<WSClientImpl>(client_)->CreateSocket(hostname, port);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSocket WSClient::CreateSocket(const std::string& hostname, int port, const WSRequestContext& request_context) {
  if (client_) {
    return static_pointer_cast<WSClientImpl>(client_)->CreateSocket(hostname, port, request_context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
