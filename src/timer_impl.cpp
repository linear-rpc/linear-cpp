#include <stdint.h>

#include "linear/event_loop.h"
#include "linear/log.h"

#include "timer_impl.h"

using namespace linear::log;

namespace linear {

static mutex g_id_mutex;

static int Id() {
  lock_guard<mutex> lock(g_id_mutex);
  static int id = 0;
  return id++;
}

TimerImpl::TimerImpl(const linear::EventLoop& loop)
  : id_(-1), state_(STOP), callback_(NULL), args_(NULL), tv_timer_(NULL),
    loop_(loop.GetImpl()) {
}

TimerImpl::TimerImpl(const linear::shared_ptr<linear::EventLoopImpl>& loop)
  : id_(-1), state_(STOP), callback_(NULL), args_(NULL), tv_timer_(NULL),
    loop_(loop) {
}

TimerImpl::~TimerImpl() {
  Stop();
}

int TimerImpl::GetId() {
  return id_;
}

Error TimerImpl::Start(TimerCallback callback, unsigned int timeout, void* args,
                       EventLoopImpl::TimerEvent* ev) {
  lock_guard<mutex> lock(mutex_);
  if (state_ == START) {
    return Error(LNR_EALREADY);
  }
  tv_timer_ = static_cast<tv_timer_t*>(malloc(sizeof(tv_timer_t)));
  if (tv_timer_ == NULL) {
    return Error(LNR_ENOMEM);
  }
  int ret = tv_timer_init(loop_->GetHandle(), tv_timer_);
  if (ret) {
    LINEAR_LOG(LOG_ERR, "fail to start timer: %s", tv_strerror(reinterpret_cast<tv_handle_t*>(tv_timer_), ret));
    free(tv_timer_);
    return Error(ret);
  }
  tv_timer_->data = ev;
  ret = tv_timer_start(tv_timer_, EventLoopImpl::OnTimer, static_cast<uint64_t>(timeout), 0);
  if (ret) {
    LINEAR_LOG(LOG_ERR, "fail to start timer: %s", tv_strerror(reinterpret_cast<tv_handle_t*>(tv_timer_), ret));
    free(tv_timer_);
    return Error(ret);
  }
  id_ = Id();
  state_ = START;
  callback_ = callback;
  args_ = args;
  return Error(LNR_OK);
}

void TimerImpl::Stop() {
  lock_guard<mutex> lock(mutex_);
  if (state_ == STOP) {
    return;
  }
  state_ = STOP;
  tv_timer_stop(tv_timer_);
  tv_close(reinterpret_cast<tv_handle_t*>(tv_timer_), EventLoopImpl::OnClose);
}

void TimerImpl::OnTimer() {
  Stop();
  if (callback_ != NULL) {
    (*callback_)(args_);
  }
}

}  // namespace linear
