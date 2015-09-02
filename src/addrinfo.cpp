#include "linear/addrinfo.h"
#include "linear/log.h"

#include "uv.h"

using namespace linear::log;

namespace linear {

Addrinfo::Addrinfo() : addr("undefined"), port(-1) {
}

Addrinfo::Addrinfo(const std::string& a, int p) : addr(a), port(p) {
}

Addrinfo::Addrinfo(const struct sockaddr* sa) : addr("undefined"), port(-1) {
  switch (sa->sa_family) {
  case AF_INET: {
    char ip_str[INET_ADDRSTRLEN];
    struct sockaddr_in* src = (struct sockaddr_in*) sa;
    int ret = uv_inet_ntop(AF_INET, &src->sin_addr, ip_str, sizeof(ip_str));
    if (ret) {
      LINEAR_LOG(LOG_ERR, "fail inet_ntop: %s", uv_strerror(ret));
    } else {
      addr = std::string(ip_str);
      port = ntohs(src->sin_port);
    }
    break;
  }
  case AF_INET6: {
    char ip_str[INET6_ADDRSTRLEN];
    struct sockaddr_in6* src = (struct sockaddr_in6*) sa;
    int ret = uv_inet_ntop(AF_INET6, &src->sin6_addr, ip_str, sizeof(ip_str));
    if (ret) {
      LINEAR_LOG(LOG_ERR, "fail inet_ntop: %s", uv_strerror(ret));
    } else {
      addr = std::string(ip_str);
      port = ntohs(src->sin6_port);
    }
    break;
  }
  default:
    break;
  }
}

Addrinfo::~Addrinfo() {
}

}  // namespace linear
