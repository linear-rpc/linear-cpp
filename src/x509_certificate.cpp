#include "linear/x509_certificate.h"

namespace linear {

X509Principal::X509Principal(X509_NAME* xname) {
  // ref: http://stackoverflow.com/questions/5136198/what-strings-are-allowed-in-the-common-name-attribute-in-an-x-509-certificate
  char buf[1024];
  if (X509_NAME_get_text_by_NID(xname, NID_commonName, buf, sizeof(buf)) != -1) {
    CN = std::string(buf);
  }
  if (X509_NAME_oneline(xname, buf, sizeof(buf))) {
    DN = std::string(buf);
  }
}

std::string X509Principal::GetCommonName() const {
  return CN;
}

class X509Certificate::X509CertificateImpl {
 public:
  explicit X509CertificateImpl(X509* xcert) {
    cert_ = X509_dup(xcert);
  }
  ~X509CertificateImpl() {
    X509_free(cert_);
  }
  bool IsCA() {
    return (X509_check_ca(cert_) > 0);
  }
  X509Principal GetSubject() {
    return X509Principal(X509_get_subject_name(cert_));
  }
  X509Principal GetIssuer() {
    return X509Principal(X509_get_issuer_name(cert_));
  }
  X509* GetHandle() {
    return cert_;
  }

private:
  X509* cert_;
};

X509Certificate::X509Certificate(X509* xcert)
  : pimpl_(new X509Certificate::X509CertificateImpl(xcert)) {
}

bool X509Certificate::IsCA() const {
  return pimpl_->IsCA();
}

X509Principal X509Certificate::GetSubject() const {
  return pimpl_->GetSubject();
}

X509Principal X509Certificate::GetIssuer() const {
  return pimpl_->GetIssuer();
}

X509* X509Certificate::GetHandle() const {
  return pimpl_->GetHandle();
}

}  // namespace linear
