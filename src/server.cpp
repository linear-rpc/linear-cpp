#include "linear/server.h"

#include "server_impl.h"

using namespace linear::log;

namespace linear {

Error Server::SetMaxClients(size_t max_clients) const {
  if (!server_) {
    return Error(LNR_EINVAL);
  }
  server_->SetMaxLimit(max_clients);
  return Error(LNR_OK);
}

Error Server::Start(const std::string& host, int port) const {
  if (!server_) {
    return Error(LNR_EINVAL);
  }
  Error e(LNR_ENOMEM);
  try {
    EventLoopImpl::ServerEvent* ev = new EventLoopImpl::ServerEvent(server_);
    e = server_->Start(host, port, ev);
    if (e != Error(LNR_OK)) {
      delete ev;
    }
  } catch(...) {
    LINEAR_LOG(LOG_ERR, "no memory");
  }
  return e;
}

Error Server::Stop() const {
  if (!server_) {
    return Error(LNR_EALREADY);
  }
  return server_->Stop();
}

} // namespace linear
