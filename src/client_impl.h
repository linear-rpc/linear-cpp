#ifndef LINEAR_CLIENT_IMPL_H_
#define LINEAR_CLIENT_IMPL_H_

#include "handler_delegate.h"

namespace linear {

class ClientImpl : public HandlerDelegate {
 public:
  ClientImpl(const linear::Handler& handler, const linear::EventLoop& loop, bool show_ssl_version = false)
    : HandlerDelegate(handler, loop, show_ssl_version) {}
  virtual ~ClientImpl() {}
};

}

#endif // LINEAR_CLIENT_IMPL_H_
