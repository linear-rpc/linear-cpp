#ifndef LINEAR_HANDLER_DELEGATE_H_
#define LINEAR_HANDLER_DELEGATE_H_

#include "linear/event_loop.h"
#include "linear/handler.h"

#include "event_loop_impl.h"
#include "socket_pool.h"

namespace linear {

class HandlerDelegate {
 public:
  HandlerDelegate(const linear::weak_ptr<linear::Handler>& handler,
                  const linear::EventLoop& loop,
                  bool show_ssl_version);
  virtual ~HandlerDelegate();

  void SetMaxLimit(size_t max_limit);
  virtual linear::Error Retain(const linear::shared_ptr<linear::SocketImpl>& socket);
  virtual void Release(const linear::shared_ptr<linear::SocketImpl>& socket);

  virtual void OnConnect(const linear::shared_ptr<linear::SocketImpl>& socket);
  virtual void OnDisconnect(const linear::shared_ptr<linear::SocketImpl>& socket,
                            const linear::Error& error);
  virtual void OnMessage(const linear::shared_ptr<linear::SocketImpl>& socket,
                         const linear::Message& message);
  virtual void OnError(const linear::shared_ptr<linear::SocketImpl>& socket,
                       const linear::Message& message,
                       const linear::Error& error);

 protected:
  linear::shared_ptr<linear::EventLoopImpl> loop_;
  linear::weak_ptr<linear::Handler> handler_;
  linear::SocketPool pool_;
};

}  // namespace linear

#endif  // LINEAR_HANDLER_DELEGATE_H_
