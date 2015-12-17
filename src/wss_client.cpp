#include "linear/wss_client.h"

#include "wss_client_impl.h"

using namespace linear::log;

namespace linear {

WSSClient::WSSClient(const shared_ptr<Handler>& handler,
                     const WSRequestContext& request_context,
                     const SSLContext& ssl_context,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, request_context, ssl_context, loop));
}

WSSClient::WSSClient(const shared_ptr<Handler>& handler,
                     const WSRequestContext& request_context,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, request_context, SSLContext(), loop));
}

WSSClient::WSSClient(const shared_ptr<Handler>& handler,
                     const SSLContext& ssl_context,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, WSRequestContext(), ssl_context, loop));
}

WSSClient::WSSClient(const shared_ptr<Handler>& handler, const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, WSRequestContext(), SSLContext(), loop));
}

void WSSClient::SetWSRequestContext(const WSRequestContext& request_context) {
  if (client_) {
    static_pointer_cast<WSSClientImpl>(client_)->SetWSRequestContext(request_context);
  }
}

void WSSClient::SetSSLContext(const SSLContext& ssl_context) {
  if (client_) {
    static_pointer_cast<WSSClientImpl>(client_)->SetSSLContext(ssl_context);
  }
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const WSRequestContext& request_context) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, request_context, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const SSLContext& ssl_context) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, ssl_context, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const WSRequestContext& request_context,
                                  const SSLContext& ssl_context) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, request_context, ssl_context, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
