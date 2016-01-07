#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

#include "tv.h"
#include "linear/log.h"

#include "auth_context_impl.h"

using namespace linear::log;

namespace linear {

static std::vector<std::string> Split(const std::string& src, char sep) {
  std::vector<std::string> vec;
  std::string s = src;

  size_t idx = 0;
  while (idx < s.length()) {
    size_t old_idx = idx;
    idx = s.find_first_of(sep, idx);
    if (idx != std::string::npos) {
      vec.push_back(s.substr(old_idx, idx - old_idx));
    } else {
      vec.push_back(s.substr(old_idx));
      break;
    }
    idx++;
  }
  return vec;
}

static std::map<std::string, std::string> Parse(const std::string& src) {
  std::map<std::string, std::string> kv;
  std::vector<std::string> vec = Split(src, ',');
  for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++) {
    std::vector<std::string> kv_vec = Split(*it, '=');
    if (kv_vec.size() != 2) {
      continue;
    }
    kv_vec[0].erase(std::remove(kv_vec[0].begin(), kv_vec[0].end(), ' '), kv_vec[0].end());
    std::transform(kv_vec[0].begin(), kv_vec[0].end(), kv_vec[0].begin(), ::tolower);
    if (kv_vec[0] == "qop" || kv_vec[0] == "nc") {
      kv_vec[1].erase(std::remove(kv_vec[1].begin(), kv_vec[1].end(), ' '), kv_vec[1].end());
    } else {
      size_t f_quote = kv_vec[1].find_first_of('"');
      size_t l_quote = kv_vec[1].find_last_of('"');
      kv_vec[1] = kv_vec[1].substr(f_quote + 1, l_quote - f_quote - 1);
    }
    kv.insert(std::make_pair(kv_vec[0], kv_vec[1]));
  }
  return kv;
}

static std::string CalcA1(const std::string& username, const std::string& realm, const std::string& password) {
  std::string A1;
  buffer a1;

  buffer_init(&a1);
  if (buffer_append(&a1, username.c_str(), username.size()) ||
      buffer_append(&a1, CONST_STRING(":")) ||
      buffer_append(&a1, realm.c_str(), realm.size()) ||
      buffer_append(&a1, CONST_STRING(":")) ||
      buffer_append(&a1, password.c_str(), password.size()) ||
      buffer_to_md5sum(&a1)) {
    buffer_fin(&a1);
    return A1;
  }
  A1 = std::string(a1.ptr, a1.len);
  buffer_fin(&a1);
  return A1;
}

static std::string CalcDigest(const std::string& A1,
                              const std::string& uri, const std::string& nonce, const std::string& nc,
                              const std::string& cnonce, const std::string& qop) {
  std::string digest;
  buffer a1, a2, rd;

  if (A1.size() <= 0) {
    return digest;
  }
  buffer_init(&a1);
  buffer_init(&a2);
  buffer_init(&rd);
  if (buffer_append(&a1, A1.c_str(), A1.size())) {
    goto go_out;
  }
  if (buffer_append(&a2, CONST_STRING("GET:")) ||
      buffer_append(&a2, uri.c_str(), uri.size()) ||
      buffer_to_md5sum(&a2)) {
    goto go_out;
  }
  if (buffer_append(&rd, a1.ptr, a1.len) ||
      buffer_append(&rd, CONST_STRING(":")) ||
      buffer_append(&rd, nonce.c_str(), nonce.size())) {
    goto go_out;
  }
  if (!nc.empty()) {
    if (buffer_append(&rd, CONST_STRING(":")) ||
        buffer_append(&rd, nc.c_str(), nc.size())) {
      goto go_out;
    }
  }
  if (!cnonce.empty()) {
    if (buffer_append(&rd, CONST_STRING(":")) ||
        buffer_append(&rd, cnonce.c_str(), cnonce.size())) {
      goto go_out;
    }
  }
  if (!qop.empty()) {
    if (buffer_append(&rd, CONST_STRING(":auth"))) {
      goto go_out;
    }
  }
  if (buffer_append(&rd, CONST_STRING(":")) ||
      buffer_append(&rd, a2.ptr, a2.len) ||
      buffer_to_md5sum(&rd)) {
    goto go_out;
  }
  digest = std::string(rd.ptr, rd.len);

go_out:
  buffer_fin(&a1);
  buffer_fin(&a2);
  buffer_fin(&rd);
  return digest;
}
  
// Authorization Header
AuthorizationContextImpl::AuthorizationContextImpl(const std::string& v) : AuthContextImpl() {
  std::string t;
  size_t loc = v.find_first_of(' ', 0);
  if (loc == std::string::npos) {
    return;
  }
  t = v.substr(0, loc);
  std::transform(t.begin(), t.end(), t.begin(), ::tolower);
  if (t == "basic") {
    type = AuthContext::BASIC;
    t = v.substr(loc + 1);
    buffer b;
    buffer_init(&b);
    if (buffer_append(&b, t.c_str(), t.size()) || buffer_from_base64(&b)) {
      LINEAR_LOG(LOG_ERR, "no memory");
      buffer_fin(&b);
      return;
    }
    std::vector<std::string> vec = Split(std::string(b.ptr, b.len), ':');
    buffer_fin(&b);
    if (vec.size() != 2) {
      return;
    }
    username = vec[0];
    response = vec[1];
  } else if (t == "digest") {
    type = AuthContext::DIGEST;
    std::map<std::string, std::string> kv = Parse(v.substr(loc));
    username = kv["username"];
    nonce = kv["nonce"];
    realm = kv["realm"];
    uri = kv["uri"];
    qop = kv["qop"];
    nc = kv["nc"];
    cnonce = kv["cnonce"];
    response = kv["response"];
  } else {
    type = AuthContext::UNKNOWN;
  }
}

AuthorizationContextImpl::~AuthorizationContextImpl() {
}

AuthorizationContext::Result AuthorizationContextImpl::Validate(const std::string& password) {
  if (type == AuthContext::BASIC) {
    return (password == response) ? AuthorizationContext::VALID : AuthorizationContext::INVALID;
  } else if (type == AuthContext::DIGEST) {
    return (username.size() > 0) ? ValidateWithHash(CalcA1(username, realm, password)) : AuthorizationContext::INVALID;
  } else if (type == AuthContext::UNKNOWN) {
    LINEAR_LOG(LOG_ERR, "unsupported authorization required");
    return AuthorizationContext::INVALID;
  }
  return AuthorizationContext::VALID;
}

AuthorizationContext::Result AuthorizationContextImpl::ValidateWithHash(const std::string& hash) {
  if (nonce.size() == 0 || uri.size() == 0 || response.size() == 0) {
    return AuthorizationContext::INVALID;
  }
  std::string digest = CalcDigest(hash, uri, nonce, nc, cnonce, qop);
  if (response == digest) {
    if (valid_nonce) {
      return AuthorizationContext::VALID;
    } else {
      LINEAR_LOG(LOG_INFO, "username and password are valid, but no valid nonce exists");
      return AuthorizationContext::AMBIGUOUS;
    }
  }
  LINEAR_LOG(LOG_WARN, "invalid authorization. username: [%s], realm: [%s], expected:[%s], received:[%s]",
             username.c_str(), realm.c_str(), digest.c_str(), response.c_str());
  return AuthorizationContext::INVALID;
}

// WWW-Authenticate Header
AuthenticateContextImpl::AuthenticateContextImpl(const std::string& v)  : AuthContextImpl(), nc(0) {
  std::string t;
  size_t loc = v.find_first_of(' ', 0);
  if (loc == std::string::npos) {
    return;
  }
  t = v.substr(0, loc);
  std::transform(t.begin(), t.end(), t.begin(), ::tolower);
  std::map<std::string, std::string> kv = Parse(v.substr(loc));
  realm = kv["realm"];
  if (t == "basic") {
    type = AuthContext::BASIC;
  } else if (t == "digest") {
    type = AuthContext::DIGEST;
    nonce = kv["nonce"];
    algorithm = kv["algorithm"];
    qop = kv["qop"];
  } else {
    type = AuthContext::UNKNOWN;
  }
}
  
AuthenticateContextImpl::~AuthenticateContextImpl() {
}

std::string AuthenticateContextImpl::CreateAuthorizationHeader(const std::string& uri, const std::string& username, const std::string& password) {
  std::string body;
  if (username.empty()) {
    return body;
  }
  if (type == AuthContext::BASIC) {
    buffer b64;
    buffer_init(&b64);
    if (buffer_append(&b64, username.c_str(), username.size()) ||
        buffer_append(&b64, CONST_STRING(":")) ||
        buffer_append(&b64, password.c_str(), password.size()) ||
        buffer_to_base64(&b64)) {
      buffer_fin(&b64);
      return body;
    }
    body = "Basic " + std::string(b64.ptr, b64.len);
    buffer_fin(&b64);
  } else if (type == AuthContext::DIGEST) {
    if (nonce.empty() || uri.empty() ||
        (!algorithm.empty() && algorithm != "MD5" && algorithm != "md5")) {
      return body;
    }
    std::string authorization = "Digest ";
    authorization = authorization + "username=\"" + username + "\", ";
    authorization = authorization + "realm=\"" + realm + "\", ";
    authorization = authorization + "uri=\"" + uri + "\", ";
    authorization = authorization + "nonce=\"" + nonce + "\", ";
    if (!algorithm.empty()) {
      authorization = authorization + "algorithm=" + algorithm + ", ";
    }
    std::string ncstr;
    std::ostringstream os;
    os.setf(std::ios::right);
    os.fill('0');
    os.width(8);
    os << std::hex << nc;
    ncstr = os.str();
    authorization = authorization + "nc=" + ncstr + ", ";

    std::string response;
    if (!qop.empty()) {
      authorization = authorization + "qop=auth, ";

      std::string cnonce;
      buffer b;
      buffer_init(&b);
      if (buffer_fill_random(&b, 24) || buffer_to_base64(&b)) {
        buffer_fin(&b);
        return body;
      }
      cnonce = std::string(b.ptr, b.len);
      buffer_fin(&b);
      authorization = authorization + "cnonce=\"" + cnonce + "\", ";
      response = CalcDigest(CalcA1(username, realm, password), uri, nonce, ncstr, cnonce, qop);
    } else {
      response = CalcDigest(CalcA1(username, realm, password), uri, nonce, ncstr, "", "");
    }
    authorization = authorization + "response=\"" + response + "\"";
    body = authorization;
  }
  return body;
}

} // namespace linear
