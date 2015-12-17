#include "linear/ssl_client.h"

#include "ssl_client_impl.h"

using namespace linear::log;

namespace linear {

SSLClient::SSLClient(const shared_ptr<Handler>& handler,
                     const SSLContext& context,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<SSLClientImpl>(new SSLClientImpl(handler, context, loop));
}

SSLClient::SSLClient(const shared_ptr<Handler>& handler,
                     const EventLoop& loop) {
  // TODO: we cannot use make_shared, delegating constructors now...
  client_ = shared_ptr<SSLClientImpl>(new SSLClientImpl(handler, SSLContext(), loop));
}

void SSLClient::SetSSLContext(const SSLContext& context) {
  if (client_) {
    static_pointer_cast<SSLClientImpl>(client_)->SetSSLContext(context);
  }
}

SSLSocket SSLClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<SSLClientImpl>(client_)->CreateSocket(hostname, port, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

SSLSocket SSLClient::CreateSocket(const std::string& hostname, int port,
                                  const SSLContext& context) {
  if (client_) {
    return static_pointer_cast<SSLClientImpl>(client_)->CreateSocket(hostname, port, context, client_);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
