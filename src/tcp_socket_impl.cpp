#include <assert.h>

#include <sstream>

#include "event_loop.h"
#include "tcp_socket_impl.h"

namespace linear {

TCPSocketImpl::TCPSocketImpl(const std::string& host, int port, const HandlerDelegate& delegate)
  : SocketImpl(host, port, delegate, Socket::TCP) {
}

TCPSocketImpl::TCPSocketImpl(tv_stream_t* stream, const HandlerDelegate& delegate)
  : SocketImpl(stream, delegate, Socket::TCP) {
}

TCPSocketImpl::~TCPSocketImpl() {
}

Error TCPSocketImpl::Connect() {
  stream_ = static_cast<tv_stream_t*>(malloc(sizeof(tv_tcp_t)));
  if (stream_ == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_tcp_init(EventLoop::GetDefault().GetHandle(), reinterpret_cast<tv_tcp_t*>(stream_));
  if (ret) {
    free(stream_);
    return Error(ret);
  }
  if (!bind_ifname_.empty()) {
    ret = tv_bindtodevice(stream_, bind_ifname_.c_str());
    if (ret) {
      free(stream_);
      return Error(ret);
    }
  }
  stream_->data = ev_;
  std::ostringstream port_str;
  port_str << peer_.port;
  ret = tv_connect(stream_, peer_.addr.c_str(), port_str.str().c_str(), EventLoop::OnConnect);
  if (ret) {
    assert(false); // never reach now
    free(stream_);
    return Error(ret);
  }
  return Error(LNR_OK);
}

}  // namespace linear
