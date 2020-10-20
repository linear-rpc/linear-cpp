#ifndef _WIN32
# include <signal.h>
#endif

#include "linear/version.h"

#include "handler_delegate.h"

#ifdef WITH_SSL
# include <openssl/crypto.h>
#endif

using namespace linear::log;

namespace linear {

HandlerDelegate::HandlerDelegate(const weak_ptr<Handler>& handler,
                                 const EventLoop& loop,
                                 bool show_ssl_version)
  : loop_(loop.GetImpl()), handler_(handler) {

#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

#ifdef WITH_SSL
  if (show_ssl_version) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s, %s",
               LINEAR_VERSION_ID, LINEAR_COMMIT_ID, SSLeay_version(SSLEAY_VERSION));
#else
    LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s, %s",
               LINEAR_VERSION_ID, LINEAR_COMMIT_ID, OpenSSL_version(OPENSSL_VERSION));
#endif
  } else
#else
  (void)(show_ssl_version);
#endif

  {
    LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s", LINEAR_VERSION_ID, LINEAR_COMMIT_ID);
  }
}

HandlerDelegate::~HandlerDelegate() {
  linear::log::DisableStderr();
  linear::log::DisableFile();
  linear::log::DisableCallback();
}

void HandlerDelegate::SetMaxLimit(size_t max) {
  pool_.SetMaxLimit(max);
}

Error HandlerDelegate::Retain(const shared_ptr<SocketImpl>& socket) {
  return pool_.Add(socket);
}

void HandlerDelegate::Release(const shared_ptr<SocketImpl>& socket) {
  return pool_.Remove(socket);
}

void HandlerDelegate::OnConnect(const shared_ptr<SocketImpl>& socket) {
  try {
    if (shared_ptr<Handler> handler = handler_.lock()) {
      handler->OnConnect(Socket(socket));
    }
  } catch(...) {
    LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnConnect");
  }
}

void HandlerDelegate::OnDisconnect(const shared_ptr<SocketImpl>& socket, const Error& error) {
  try {
    if (shared_ptr<Handler> handler = handler_.lock()) {
      handler->OnDisconnect(Socket(socket), error);
    }
  } catch(...) {
    LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnDisconnect");
  }
}

void HandlerDelegate::OnMessage(const shared_ptr<SocketImpl>& socket, const Message& message) {
  if (message.type == RESPONSE) {
    Response response = message.as<Response>();
    const Request& request = response.request;
    if (request.HasResponseCallback()) {
      try {
        request.FireResponseCallback(Socket(socket), response);
      } catch(...) {
        LINEAR_LOG(LOG_WARN, "something wrong at OnMessage closure");
      }
    } else {
      try {
        if (shared_ptr<Handler> handler = handler_.lock()) {
          handler->OnMessage(Socket(socket), message);
        }
      } catch(...) {
        LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnMessage");
      }
    }
  } else {
    try {
      if (shared_ptr<Handler> handler = handler_.lock()) {
        handler->OnMessage(Socket(socket), message);
      }
    } catch(...) {
      LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnMessage");
    }
  }
}

void HandlerDelegate::OnError(const shared_ptr<SocketImpl>& socket, const Message& message, const Error& error) {
  if (message.type == REQUEST) {
    Request request = message.as<Request>();
    if (request.HasErrorCallback()) {
      try {
        request.FireErrorCallback(Socket(socket), request, error);
      } catch(...) {
        LINEAR_LOG(LOG_WARN, "something wrong at OnError closure");
      }
    } else {
      try {
        if (shared_ptr<Handler> handler = handler_.lock()) {
          handler->OnError(Socket(socket), message, error);
        }
      } catch(...) {
        LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnError");
      }
    }
  } else {
    try {
      if (shared_ptr<Handler> handler = handler_.lock()) {
        handler->OnError(Socket(socket), message, error);
      }
    } catch(...) {
      LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnError");
    }
  }
}

} // namespace linear
