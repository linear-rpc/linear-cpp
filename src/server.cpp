#include "linear/server.h"

#include "server_impl.h"

namespace linear {

Error Server::SetMaxClients(int max_clients) {
  if (!server_) {
    return Error(LNR_EINVAL);
  }
  server_->SetMaxLimit(max_clients);
  return Error(LNR_OK);
}

Error Server::Start(const std::string& host, int port) {
  if (!server_) {
    return Error(LNR_EINVAL);
  }
  return server_->Start(host, port);
}

Error Server::Stop() {
  if (!server_) {
    return Error(LNR_EALREADY);
  }
  return server_->Stop();
}

} // namespace linear
