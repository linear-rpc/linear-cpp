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
    handler_delegate_observer_(new Observer<HandlerDelegate>()) {

#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif

#ifdef WITH_SSL
  if (show_ssl_version) {
    LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s, %s", LINEAR_VERSION_ID, LINEAR_COMMIT_ID, SSLeay_version(SSLEAY_VERSION));
  } else {
    LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s", LINEAR_VERSION_ID, LINEAR_COMMIT_ID);
  }
#else
  (void)(show_ssl_version);
  LINEAR_LOG(LOG_DEBUG, "version: %s, sign: %s", LINEAR_VERSION_ID, LINEAR_COMMIT_ID);
#endif

  handler_delegate_observer_->Lock();
  handler_delegate_observer_->Register(this);
  handler_delegate_observer_->Unlock();
}

HandlerDelegate::~HandlerDelegate() {
  handler_delegate_observer_->Lock();
  handler_delegate_observer_->Unregister();
  handler_delegate_observer_->Unlock();
}

const shared_ptr<Observer<HandlerDelegate> >& HandlerDelegate::GetObserver() const {
  return handler_delegate_observer_;
}

void HandlerDelegate::SetMaxLimit(int max) {
  pool_.SetMaxLimit(max);
}

Error HandlerDelegate::Retain(const Socket& socket) {
  return pool_.Add(socket);
}

const linear::Socket& HandlerDelegate::Get(int id) {
  return pool_.Get(id);
}

void HandlerDelegate::Release(int id) {
  pool_.Remove(id);
}

void HandlerDelegate::OnConnect(int id) {
  handler_observer_->Lock();
  Handler* handler = handler_observer_->GetSubject();
  if (handler) {
    const Socket& socket = pool_.Get(id);
    try {
      handler->OnConnect(socket);
    } catch(...) {
      LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnConnect");
    }
  }
  handler_observer_->Unlock();
}

void HandlerDelegate::OnDisconnect(const linear::Socket& socket, const Error& error) {
  handler_observer_->Lock();
  Handler* handler = handler_observer_->GetSubject();
  if (handler) {
    try {
      handler->OnDisconnect(socket, error);
    } catch(...) {
      LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnDisconnect");
    }
  }
  handler_observer_->Unlock();
}

void HandlerDelegate::OnMessage(int id, const Message& message) {
  handler_observer_->Lock();
  Handler* handler = handler_observer_->GetSubject();
  if (handler) {
    const Socket& socket = pool_.Get(id);
    if (message.type == RESPONSE) {
      Response response = message.as<Response>();
      const Request& request = response.request;
      if (request.HasResponseCallback()) {
        try {
          request.FireResponseCallback(socket, response);
        } catch(...) {
          LINEAR_LOG(LOG_WARN, "something wrong at OnMessage closure");
        }
      } else {
        try {
          handler->OnMessage(socket, message);
        } catch(...) {
          LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnMessage");
        }
      }
    } else {
      try {
        handler->OnMessage(socket, message);
      } catch(...) {
          LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnMessage");
      }
    }
  }
  handler_observer_->Unlock();
}

void HandlerDelegate::OnError(const linear::Socket& socket, const Message& message, const Error& error) {
  handler_observer_->Lock();
  Handler* handler = handler_observer_->GetSubject();
  if (handler) {
    if (message.type == REQUEST) {
      Request request = message.as<Request>();
      if (request.HasErrorCallback()) {
        try {
          request.FireErrorCallback(socket, request, error);
        } catch(...) {
          LINEAR_LOG(LOG_WARN, "something wrong at OnError closure");
        }
      } else {
        try {
          handler->OnError(socket, message, error);
        } catch(...) {
          LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnError");
        }
      }
    } else {
      try {
        handler->OnError(socket, message, error);
      } catch(...) {
        LINEAR_LOG(LOG_WARN, "something wrong at Handler::OnError");
      }
    }
  }
  handler_observer_->Unlock();
}

} // namespace linear
