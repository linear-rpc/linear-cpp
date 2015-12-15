#ifndef LINEAR_HANDLER_DELEGATE_H_
#define LINEAR_HANDLER_DELEGATE_H_

#include "linear/event_loop.h"
#include "linear/handler.h"

#include "event_loop_impl.h"
#include "socket_pool.h"

namespace linear {

class HandlerDelegate {
 public:
  HandlerDelegate(const linear::Handler& handler, const linear::EventLoop& loop, bool show_ssl_version);
  virtual ~HandlerDelegate();
  linear::weak_ptr<linear::Observer<linear::HandlerDelegate> > GetObserver() const;
  void SetMaxLimit(size_t max_limit);
  linear::Error Retain(const shared_ptr<SocketImpl>& socket);
  void Release(const shared_ptr<SocketImpl>& socket);
  virtual void OnConnect(const shared_ptr<SocketImpl>& socket);
  virtual void OnDisconnect(const shared_ptr<SocketImpl>& socket, const linear::Error& error);
  virtual void OnMessage(const shared_ptr<SocketImpl>& socket, const linear::Message& message);
  virtual void OnError(const shared_ptr<SocketImpl>& socket, const linear::Message& message, const linear::Error& error);

 protected:
  linear::shared_ptr<linear::EventLoopImpl> loop_;
  linear::weak_ptr<linear::Observer<linear::Handler> > handler_observer_;
  linear::shared_ptr<linear::Observer<linear::HandlerDelegate> > handler_delegate_observer_;
  linear::SocketPool pool_;
};

}  // namespace linear

#endif  // LINEAR_HANDLER_DELEGATE_H_
