#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "linear/log.h"

#include "event_loop.h"
#include "timer_impl.h"

using namespace linear::log;

namespace linear {

TimerImpl::TimerImpl() : state_(STOP), tv_timer_(NULL), callback_(NULL), args_(NULL), mutex_() {
}

TimerImpl::~TimerImpl() {
}

Error TimerImpl::Start(int id, TimerCallback callback, unsigned int timeout, void* args) {
  linear::lock_guard<linear::mutex> lock(mutex_);
  if (state_ == START) {
    return Error(LNR_EALREADY);
  }
  tv_timer_ = static_cast<tv_timer_t*>(malloc(sizeof(tv_timer_t)));
  if (tv_timer_ == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_timer_init(EventLoop::GetDefault().GetHandle(), tv_timer_);
  if (ret) {
    LINEAR_LOG(LOG_ERR, "fail to start timer: %s", tv_strerror(reinterpret_cast<tv_handle_t*>(tv_timer_), ret));
    free(tv_timer_);
    tv_timer_ = NULL;
    return Error(ret);
  }
  EventLoop::TimerEventData* data = new EventLoop::TimerEventData(id, this);
  tv_timer_->data = data;
  state_ = START;
  callback_ = callback;
  args_ = args;
  ret = tv_timer_start(tv_timer_, EventLoop::OnTimer, static_cast<uint64_t>(timeout), 0);
  if (ret) {
    LINEAR_LOG(LOG_ERR, "fail to start timer: %s", tv_strerror(reinterpret_cast<tv_handle_t*>(tv_timer_), ret));
    state_ = STOP;
    delete data;
    free(tv_timer_);
    tv_timer_ = NULL;
    return Error(ret);
  }
  LINEAR_DEBUG(LOG_DEBUG, "timer = %p, timer->data = %p", tv_timer_, tv_timer_->data);
  return Error(LNR_OK);
}

void TimerImpl::Stop() {
  linear::lock_guard<linear::mutex> lock(mutex_);
  if (state_ == STOP) {
    return;
  }
  EventLoop::TimerEventData* data = static_cast<EventLoop::TimerEventData*>(tv_timer_->data);
  data->Lock();
  data->Unregister();
  data->Unlock();
  state_ = STOP;
  tv_timer_stop(tv_timer_);
  tv_close(reinterpret_cast<tv_handle_t*>(tv_timer_), EventLoop::OnClose);
  tv_timer_ = NULL;
}

void TimerImpl::OnTimer() {
  linear::unique_lock<linear::mutex> lock(mutex_);
  if (state_ == STOP) {
    return;
  }
  state_ = STOP;
  tv_timer_stop(tv_timer_);
  tv_close(reinterpret_cast<tv_handle_t*>(tv_timer_), EventLoop::OnClose);
  tv_timer_ = NULL;
  lock.unlock();
  if (callback_ != NULL) {
    (*callback_)(args_);
  }
}

}  // namespace linear
