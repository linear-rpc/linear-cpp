#include "linear/ws_client.h"

#include "ws_client_impl.h"

using namespace linear::log;

namespace linear {

WSClient::WSClient(const shared_ptr<Handler>& handler,
                   const WSRequestContext& request_context,
                   const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<WSClientImpl>(new WSClientImpl(handler, request_context, loop));
}

WSClient::WSClient(const shared_ptr<Handler>& handler,
                   const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<WSClientImpl>(new WSClientImpl(handler, WSRequestContext(), loop));
}

void WSClient::SetWSRequestContext(const WSRequestContext& request_context) {
  if (client_) {
    static_pointer_cast<WSClientImpl>(client_)->SetWSRequestContext(request_context);
  }
}

WSSocket WSClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<WSClientImpl>(client_)->CreateSocket(hostname, port, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSocket WSClient::CreateSocket(const std::string& hostname, int port,
                                const WSRequestContext& request_context) {
  if (client_) {
    return static_pointer_cast<WSClientImpl>(client_)->CreateSocket(hostname, port, request_context, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
