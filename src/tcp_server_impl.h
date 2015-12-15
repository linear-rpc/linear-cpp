#ifndef LINEAR_TCP_SERVER_IMPL_H_
#define LINEAR_TCP_SERVER_IMPL_H_

#include "server_impl.h"

namespace linear {

class TCPServerImpl : public ServerImpl {
 public:
  TCPServerImpl(const linear::Handler& handler, const linear::EventLoop& loop);
  virtual ~TCPServerImpl();
  linear::Error Start(const std::string& hostname, int port,
                      linear::EventLoopImpl::ServerEvent* ev);
  linear::Error Stop();
  void OnAccept(tv_stream_t* srv_stream, tv_stream_t* cli_stream, int status);

 private:
  tv_tcp_t* handle_;
};

}  // namespace linear

#endif  // LINEAR_TCP_SERVER_IMPL_H_
