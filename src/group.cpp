#include <map>

#include "linear/mutex.h"
#include "linear/log.h"
#include "linear/group.h"

using namespace linear::log;

namespace linear {

namespace group {

class Pool {
 public:
  static Pool& GetInstance() {
    static Pool pool;
    return pool;
  }
  Pool() : mutex_() {}
  ~Pool() {}
  std::vector<std::string> Names() {
    std::vector<std::string> keys;
    lock_guard<linear::mutex> lock(mutex_);

    std::map<std::string, std::set<linear::Socket> >::iterator it = pool_.begin();
    while(it != pool_.end()) {
      keys.push_back(it->first);
      ++it;
    }
    return keys;
  }
  std::set<linear::Socket> Get(const std::string& name) {
    lock_guard<linear::mutex> lock(mutex_);
    std::map<std::string, std::set<linear::Socket> >::iterator it = pool_.find(name);
    if (it != pool_.end()) {
      return it->second;
    } else {
      return std::set<linear::Socket>();
    }
  }
  void Join(const std::string& name, const linear::Socket& socket) {
    lock_guard<linear::mutex> lock(mutex_);
    LINEAR_LOG(LOG_DEBUG, "join socket(id = %d) into group_name = \"%s\"",
               socket.GetId(), name.c_str());
    pool_[name].insert(socket);
  }
  void Leave(const std::string& name, const linear::Socket& socket) {
    lock_guard<linear::mutex> lock(mutex_);
    LINEAR_LOG(LOG_DEBUG, "leave socket(id = %d) from group_name = \"%s\"",
               socket.GetId(), name.c_str());
    pool_[name].erase(socket);
  }
  void Leave(const linear::Socket& socket) {
    lock_guard<linear::mutex> lock(mutex_);
    std::map<std::string, std::set<linear::Socket> >::iterator pool_it = pool_.begin();
    while (pool_it != pool_.end()) {
      std::set<linear::Socket>& sockets = (*pool_it).second;
      std::set<linear::Socket>::iterator socket_it = sockets.begin();
      while (socket_it != sockets.end()) {
        if (socket.GetId() == (*socket_it).GetId()) {
          LINEAR_LOG(LOG_DEBUG, "leave socket(id = %d) from group_name = \"%s\"",
                     socket.GetId(), (*pool_it).first.c_str());
          sockets.erase(*socket_it);
          break;
        } else {
          socket_it++;
        }
      }
      pool_it++;
    }
  }

 private:
  Pool(const Pool& pool);
  Pool& operator=(const Group& pool);
  std::map<std::string, std::set<linear::Socket> >pool_;
  linear::mutex mutex_;
};

} // namespace group

std::vector<std::string> Group::Names() {
  group::Pool& pool = group::Pool::GetInstance();
  return pool.Names();
}

std::set<linear::Socket> Group::Get(const std::string& name) {
  group::Pool& pool = group::Pool::GetInstance();
  return pool.Get(name);
}

void Group::Join(const std::string& name, const linear::Socket& socket) {
  group::Pool& pool = group::Pool::GetInstance();
  return pool.Join(name, socket);
}

void Group::Leave(const std::string& name, const linear::Socket& socket) {
  group::Pool& pool = group::Pool::GetInstance();
  return pool.Leave(name, socket);
}

void Group::LeaveAll(const linear::Socket& socket) {
  group::Pool& pool = group::Pool::GetInstance();
  return pool.Leave(socket);
}

}  // namespace linear
