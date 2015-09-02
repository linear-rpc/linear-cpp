#include "linear/log.h"
#include "linear/handler.h"

namespace linear {

using namespace linear::log;

Handler::Handler() : observer_(new Observer<Handler>()) {
  observer_->Lock();
  observer_->Register(this);
  observer_->Unlock();
}

Handler::~Handler() {
  observer_->Lock();
  observer_->Unregister();
  observer_->Unlock();
}

shared_ptr<Observer<Handler> > Handler::GetObserver() const {
  return observer_;
}

} // namespace linear
