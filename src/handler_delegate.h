#ifndef LINEAR_HANDLER_DELEGATE_H_
#define LINEAR_HANDLER_DELEGATE_H_

#include "linear/handler.h"

#include "socket_pool.h"

namespace linear {

class HandlerDelegate {
 public:
  HandlerDelegate(const linear::Handler& handler, bool show_ssl_version);
  virtual ~HandlerDelegate();
  linear::weak_ptr<linear::Observer<linear::HandlerDelegate> > GetObserver() const;

  virtual void SetMaxLimit(size_t max_limit);
  virtual linear::Error Retain(const shared_ptr<SocketImpl>& socket);
  virtual void Release(const shared_ptr<SocketImpl>& socket);
  virtual void OnConnect(const shared_ptr<SocketImpl>& socket);
  virtual void OnDisconnect(const shared_ptr<SocketImpl>& socket, const linear::Error& error);
  virtual void OnMessage(const shared_ptr<SocketImpl>& socket, const linear::Message& message);
  virtual void OnError(const shared_ptr<SocketImpl>& socket, const linear::Message& message, const linear::Error& error);

 protected:
  linear::weak_ptr<linear::Observer<linear::Handler> > handler_observer_;
  linear::shared_ptr<linear::Observer<linear::HandlerDelegate> > handler_delegate_observer_;
  linear::SocketPool pool_;
};

}  // namespace linear

#endif  // LINEAR_HANDLER_DELEGATE_H_
