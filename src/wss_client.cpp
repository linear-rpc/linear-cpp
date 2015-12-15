#include "linear/wss_client.h"

#include "wss_client_impl.h"

using namespace linear::log;

namespace linear {

WSSClient::WSSClient(const Handler& handler,
                     const linear::WSRequestContext& request_context,
                     const linear::SSLContext& ssl_context,
                     const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, request_context, ssl_context, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSSClient::WSSClient(const Handler& handler,
                     const linear::WSRequestContext& request_context,
                     const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, request_context, linear::SSLContext(), loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSSClient::WSSClient(const Handler& handler,
                     const linear::SSLContext& ssl_context,
                     const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, linear::WSRequestContext(), ssl_context, loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

WSSClient::WSSClient(const Handler& handler, const linear::EventLoop& loop) {
  try {
    client_ = shared_ptr<WSSClientImpl>(new WSSClientImpl(handler, linear::WSRequestContext(), linear::SSLContext(), loop));
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
    throw;
  }
}

void WSSClient::SetWSRequestContext(const linear::WSRequestContext& request_context) {
  if (client_) {
    static_pointer_cast<WSSClientImpl>(client_)->SetWSRequestContext(request_context);
  }
}

void WSSClient::SetSSLContext(const linear::SSLContext& ssl_context) {
  if (client_) {
    static_pointer_cast<WSSClientImpl>(client_)->SetSSLContext(ssl_context);
  }
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const linear::WSRequestContext& request_context) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, request_context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const linear::SSLContext& ssl_context) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, ssl_context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

WSSSocket WSSClient::CreateSocket(const std::string& hostname, int port,
                                  const linear::WSRequestContext& request_context,
                                  const linear::SSLContext& ssl_context) {
  if (client_) {
    return static_pointer_cast<WSSClientImpl>(client_)->CreateSocket(hostname, port, request_context, ssl_context);
  }
  LINEAR_LOG(LOG_ERR, "handler is not set");
  throw std::invalid_argument("handler is not set");
}

}  // namespace linear
