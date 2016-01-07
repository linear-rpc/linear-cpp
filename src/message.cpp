#include "linear/log.h"
#include "linear/mutex.h"
#include "linear/message.h"
#include "linear/group.h"

using namespace linear::log;

namespace linear {

static linear::mutex g_id_mutex;

static uint32_t GetId() {
  linear::lock_guard<linear::mutex> lock(g_id_mutex);
  static uint32_t id = 0;
  return ++id;
}

Request::Request() : Message(linear::REQUEST), msgid(GetId()), timeout_(30000) {
}

Request::Request(const std::string& m, const type::any& p)
  : Message(linear::REQUEST), msgid(GetId()), method(m), params(p), timeout_(30000) {
}

Error Request::Send(const Socket& socket) const {
  return socket.Send(*this, this->timeout_);
}

Error Request::Send(const Socket& socket, int timeout) const {
  Request* request = const_cast<Request*>(this);
  request->timeout_ = timeout;
  return Send(socket);
}

bool Request::HasResponseCallback() const {
  return (bool) on_response_holder_;
}
bool Request::HasErrorCallback() const {
  return (bool) on_error_holder_;
}

void Request::FireResponseCallback(const Socket& socket, const Response& response) const {
  on_response_holder_->Fire(socket, response);
}
void Request::FireErrorCallback(const Socket& socket, const Request& request, const Error& error) const {
  on_error_holder_->Fire(socket, request, error);
}

Error Response::Send(const Socket& socket) const {
  return socket.Send(*this, 0);
}

Error Notify::Send(const Socket& socket) const {
  return socket.Send(*this, 0);
}

void Notify::Send(const std::string& group_name) const {
  std::set<linear::Socket> sockets = Group::Get(group_name);
  if (sockets.empty()) {
    return;
  }
  if (group_name == std::string(LINEAR_BROADCAST_GROUP)) {
    LINEAR_LOG(LOG_DEBUG, "Send to broadcast group");
  } else {
    LINEAR_LOG(LOG_DEBUG, "Send to group: \"%s\"", group_name.c_str());
  }
  std::set<linear::Socket>::iterator it = sockets.begin();
  while (it != sockets.end()) {
    (*it).Send(*this, 0);
    it++;
  }
}

void Notify::Send(const std::string& group_name, const Socket& except_socket) const {
  std::set<linear::Socket> sockets = Group::Get(group_name);
  if (sockets.empty()) {
    return;
  }
  if (group_name == std::string(LINEAR_BROADCAST_GROUP)) {
    LINEAR_LOG(LOG_DEBUG, "Send to broadcast group except for socket(id = %d)",
               except_socket.GetId());
  } else {
    LINEAR_LOG(LOG_DEBUG, "Send to group: \"%s\" except for socket(id = %d)",
               group_name.c_str(), except_socket.GetId());
  }
  std::set<linear::Socket>::iterator it = sockets.begin();
  while (it != sockets.end()) {
    if ((*it) != except_socket) {
      (*it).Send(*this, 0);
    }
    it++;
  }
}

}  // namespace linear
