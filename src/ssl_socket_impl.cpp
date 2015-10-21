#include <assert.h>

#include <sstream>

#include "event_loop.h"
#include "ssl_socket_impl.h"

using namespace linear::log;

namespace linear {

SSLSocketImpl::SSLSocketImpl(const std::string& host, int port, const linear::SSLContext& context,
                             const linear::HandlerDelegate& delegate)
  : SocketImpl(host, port, delegate, Socket::SSL), context_(context) {
}

SSLSocketImpl::SSLSocketImpl(tv_stream_t* stream, const SSLContext& context,
                             const linear::HandlerDelegate& delegate)
  : SocketImpl(stream, delegate, Socket::SSL), context_(context) {
}

SSLSocketImpl::~SSLSocketImpl() {
}

Error SSLSocketImpl::Connect() {
  stream_ = static_cast<tv_stream_t*>(malloc(sizeof(tv_ssl_t)));
  if (stream_ == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_ssl_init(EventLoop::GetDefault().GetHandle(), reinterpret_cast<tv_ssl_t*>(stream_), context_.GetHandle());
  if (ret) {
    free(stream_);
    stream_ = NULL;
    return Error(ret);
  }
  data_ = new EventLoop::SocketEventData();
  data_->Register(this);
  stream_->data = data_;
  std::ostringstream port_str;
  port_str << peer_.port;
  if (!bind_ifname_.empty()) {
    ret = tv_bindtodevice(stream_, bind_ifname_.c_str());
    if (ret != 0) {
      LINEAR_LOG(LOG_ERR, "SO_BINDTODEVICE failed(%d)", ret);
      return Error(ret);
    }
  }
  ret = tv_connect(stream_, peer_.addr.c_str(), port_str.str().c_str(), EventLoop::OnConnect);
  if (ret) {
    assert(false); // never reach now
    delete data_;
    data_ = NULL;
    stream_->data = NULL;
    free(stream_);
    stream_ = NULL;
    return Error(ret);
  }
  return Error(LNR_OK);
}

Error SSLSocketImpl::GetVerifyResult() {
  lock_guard<mutex> state_lock(state_mutex_);
  if (state_ != Socket::CONNECTING && state_ != Socket::CONNECTED) {
    return Error(LNR_ENOTCONN);
  }
  int ret = tv_ssl_get_verify_result(reinterpret_cast<tv_ssl_t*>(stream_));
  if (ret) {
    return Error(ret, stream_->ssl_err);
  } else {
    return Error(LNR_OK);
  }
}

bool SSLSocketImpl::PresentPeerCertificate() {
  lock_guard<mutex> state_lock(state_mutex_);
  if (state_ != Socket::CONNECTING && state_ != Socket::CONNECTED) {
    return false;
  }
  X509* xcert = tv_ssl_get_peer_certificate(reinterpret_cast<tv_ssl_t*>(stream_));
  if (xcert == NULL) {
    return false;
  }
  X509_free(xcert);
  return true;
}

X509Certificate SSLSocketImpl::GetPeerCertificate() {
  lock_guard<mutex> state_lock(state_mutex_);
  if (state_ != Socket::CONNECTING && state_ != Socket::CONNECTED) {
    throw std::runtime_error("peer certificate does not exist");
  }
  X509* xcert = tv_ssl_get_peer_certificate(reinterpret_cast<tv_ssl_t*>(stream_));
  if (xcert == NULL) {
    throw std::runtime_error("peer certificate does not exist");
  }
  return X509Certificate(xcert);
}

}  // namespace linear
