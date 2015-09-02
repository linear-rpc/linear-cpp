#ifndef LINEAR_SERVER_IMPL_H_
#define LINEAR_SERVER_IMPL_H_

#include "tv.h"

#include <string>

#include "linear/group.h"

#include "event_loop.h"
#include "handler_delegate.h"

namespace linear {

class ServerImpl : public HandlerDelegate {
 public:
  static const int BACKLOG = 10; // backlog = 10

 public:
  ServerImpl(const linear::Handler& handler, bool show_ssl_version = false) : HandlerDelegate(handler, show_ssl_version) {}
  virtual ~ServerImpl() {}

  virtual linear::Error Start(const std::string& hostname, int port) = 0;
  virtual linear::Error Stop() = 0;
  virtual void Release(int id) {
    Group::LeaveAll(pool_.Get(id));
    this->HandlerDelegate::Release(id);
  }
  virtual void OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status) = 0;

 protected:
  linear::Addrinfo self_;
  linear::mutex mutex_;
};

}  // namespace linear

#endif  // LINEAR_SERVER_H_
