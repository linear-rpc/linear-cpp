#include "linear/wss_client.h"

#include "wss_client_impl.h"

using namespace linear::log;

namespace linear {

WSSClient::WSSClient(const Handler& handler,
                     const linear::WSRequestContext& request_context,
                     const linear::SSLContext& ssl_context) {
  try {
    client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, request_context, ssl_context));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSSClient::~WSSClient() {
}

void WSSClient::SetWSRequestContext(const linear::WSRequestContext& request_context) {
  if (client_) {
    dynamic_pointer_cast<WSSClientImpl>(client_)->SetWSRequestContext(request_context);
  }
}

void WSSClient::SetSSLContext(const linear::SSLContext& ssl_context) {
  if (client_) {
    dynamic_pointer_cast<WSSClientImpl>(client_)->SetSSLContext(ssl_context);
  }
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return dynamic_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const linear::WSRequestContext& request_context) {
  if (client_) {
    return dynamic_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, request_context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const linear::WSRequestContext& request_context,
                                  const linear::SSLContext& ssl_context) {
  if (client_) {
    return dynamic_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, request_context, ssl_context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
