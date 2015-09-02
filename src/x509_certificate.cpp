#include "linear/x509_certificate.h"

namespace linear {

X509Principal::X509Principal(X509_NAME* xname) {
  char buf[1024];
  if (X509_NAME_get_text_by_NID(xname, NID_commonName, buf, sizeof(buf)) != -1) {
    common_name_ = std::string(buf);
  }
}

std::string X509Principal::GetCommonName() const {
  return common_name_;
}


X509Certificate::X509Certificate(X509* xcert)
  : subject_(X509_get_subject_name(xcert)), issuer_(X509_get_issuer_name(xcert)) {
  X509_free(xcert);
}

X509Principal X509Certificate::GetSubject() const {
  return subject_;
}

X509Principal X509Certificate::GetIssuer() const {
  return issuer_;
}

}  // namespace linear
