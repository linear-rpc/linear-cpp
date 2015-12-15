#include "linear/ssl_client.h"

#include "ssl_client_impl.h"

using namespace linear::log;

namespace linear {

SSLClient::SSLClient(const Handler& handler, const SSLContext& context,
                     const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<SSLClientImpl>(new SSLClientImpl(handler, context, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

SSLClient::SSLClient(const Handler& handler, const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<SSLClientImpl>(new SSLClientImpl(handler, linear::SSLContext(), loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

void SSLClient::SetSSLContext(const SSLContext& context) {
  if (client_) {
    static_pointer_cast<SSLClientImpl>(client_)->SetSSLContext(context);
  }
}

SSLSocket SSLClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<SSLClientImpl>(client_)->CreateSocket(hostname, port);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

SSLSocket SSLClient::CreateSocket(const std::string& hostname, int port, const SSLContext& context) {
  if (client_) {
    return static_pointer_cast<SSLClientImpl>(client_)->CreateSocket(hostname, port, context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
