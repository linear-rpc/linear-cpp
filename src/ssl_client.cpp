#include <stdexcept>

#include "linear/log.h"
#include "linear/ssl_client.h"

#include "ssl_client_impl.h"

using namespace linear::log;

namespace linear {

SSLClient::SSLClient(const Handler& handler, const SSLContext& context) {
  try {
    client_ = shared_ptr<SSLClientImpl>(new SSLClientImpl(handler, context));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

SSLClient::~SSLClient() {
}

void SSLClient::SetContext(const SSLContext& context) {
  if (client_) {
    dynamic_pointer_cast<SSLClientImpl>(client_)->SetContext(context);
  }
}

SSLSocket SSLClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return dynamic_pointer_cast<SSLClientImpl>(client_)->CreateSocket(hostname, port);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

SSLSocket SSLClient::CreateSocket(const std::string& hostname, int port, const SSLContext& context) {
  if (client_) {
    return dynamic_pointer_cast<SSLClientImpl>(client_)->CreateSocket(hostname, port, context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
