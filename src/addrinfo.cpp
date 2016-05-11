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
  socklen_t slen = 0;
  switch (sa->sa_family) {
  case AF_INET:
    {
      const struct sockaddr_in* src = reinterpret_cast<const struct sockaddr_in*>(sa);
      port = ntohs(src->sin_port);
      slen = sizeof(struct sockaddr_in);
      break;
    }
  case AF_INET6:
    {
      const struct sockaddr_in6* src = reinterpret_cast<const struct sockaddr_in6*>(sa);
      port = ntohs(src->sin6_port);
      slen = sizeof(struct sockaddr_in6);
      break;
    }
  default:
    return;
  }
  char host[NI_MAXHOST];
  if (getnameinfo(sa, slen, host, sizeof(host), NULL, 0, NI_NUMERICHOST) == 0) {
    addr = std::string(host);
  }
}

Addrinfo::~Addrinfo() {
}

}  // namespace linear
