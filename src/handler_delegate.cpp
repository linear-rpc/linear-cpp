#ifndef _WIN32
# include <signal.h>
#endif

#ifdef WITH_SSL
# include <openssl/crypto.h>
#endif

#include "linear/log.h"
#include "linear/version.h"

#include "handler_delegate.h"

using namespace linear::log;

namespace linear {

HandlerDelegate::HandlerDelegate(const Handler& handler, bool show_ssl_version)
  : handler_observer_(handler.GetObserver()),
    handler_delegate_observer_(new Observer<HandlerDelegate>(this)) {
  static bool shown(false);

#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

  if (!shown) {
#ifdef WITH_SSL
    if (show_ssl_version) {
      LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s, %s",
                 LINEAR_VERSION_ID, LINEAR_COMMIT_ID, SSLeay_version(SSLEAY_VERSION));
    } else
#else
      (void)(show_ssl_version);
#endif

    {
      LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s", LINEAR_VERSION_ID, LINEAR_COMMIT_ID);
    }
    shown = true;
  }
}

HandlerDelegate::~HandlerDelegate() {
  handler_delegate_observer_->Terminate();
}

weak_ptr<Observer<HandlerDelegate> > HandlerDelegate::GetObserver() const {
  return handler_delegate_observer_;
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
  if (shared_ptr<Observer<Handler> > observer = handler_observer_.lock()) {
    observer->Lock();
    Handler* handler = observer->GetSubject();
    if (handler) {
      try {
        handler->OnConnect(Socket(socket));
      } catch(...) {
        LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnConnect");
      }
    }
    observer->Unlock();
  }
}

void HandlerDelegate::OnDisconnect(const shared_ptr<SocketImpl>& socket, const Error& error) {
  if (shared_ptr<Observer<Handler> > observer = handler_observer_.lock()) {
    observer->Lock();
    Handler* handler = observer->GetSubject();
    if (handler) {
      try {
        handler->OnDisconnect(Socket(socket), error);
      } catch(...) {
        LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnDisconnect");
      }
    }
    observer->Unlock();
  }
}

void HandlerDelegate::OnMessage(const shared_ptr<SocketImpl>& socket, const Message& message) {
  if (shared_ptr<Observer<Handler> > observer = handler_observer_.lock()) {
    observer->Lock();
    Handler* handler = observer->GetSubject();
    if (handler) {
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
            handler->OnMessage(Socket(socket), message);
          } catch(...) {
            LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnMessage");
          }
        }
      } else {
        try {
          handler->OnMessage(Socket(socket), message);
        } catch(...) {
          LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnMessage");
        }
      }
    }
    observer->Unlock();
  }
}

void HandlerDelegate::OnError(const shared_ptr<SocketImpl>& socket, const Message& message, const Error& error) {
  if (shared_ptr<Observer<Handler> > observer = handler_observer_.lock()) {
    observer->Lock();
    Handler* handler = observer->GetSubject();
    if (handler) {
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
            handler->OnError(Socket(socket), message, error);
          } catch(...) {
            LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnError");
          }
        }
      } else {
        try {
          handler->OnError(Socket(socket), message, error);
        } catch(...) {
          LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnError");
        }
      }
    }
    observer->Unlock();
  }
}

} // namespace linear
