#include "tv.h"

#include "linear/error.h"

#ifdef WITH_SSL
# include <openssl/err.h>
# include <openssl/x509.h>
#endif

namespace linear {

Error::Error() : code_(LNR_UNKNOWN), detail_code_(0) {
}

Error::Error(linear::ErrorCode code, unsigned long detail_code) : code_(code), detail_code_(detail_code) {
}

Error::Error(int code, unsigned long detail_code) : detail_code_(detail_code) {
#define LNR_ERR_NAME_GEN(name, s) case TV_##name : code_ = LNR_##name; break;
  switch (code) {
  case 0:
    code_ = LNR_OK;
    break;
  LNR_ERRNO_MAP(LNR_ERR_NAME_GEN)
  default:
    code_ = LNR_UNKNOWN;
  }
#undef LNR_ERR_NAME_GEN
}

Error::~Error() {
}

bool Error::operator==(const linear::Error& err) const {
  return (code_ == err.code_) && (detail_code_ == err.detail_code_);
}

bool Error::operator!=(const linear::Error& err) const {
  return (code_ != err.code_) || (detail_code_ != err.detail_code_);
}

const linear::ErrorCode& Error::Code() const {
  return code_;
}

unsigned long Error::DetailCode() const {
  return detail_code_;
}

std::string Error::Message() const {
#ifdef WITH_SSL
  if (code_ == LNR_EX509 && detail_code_) {
    return X509_verify_cert_error_string(detail_code_);
  } else if (code_ == LNR_ESSL && detail_code_) {
    return ERR_error_string(detail_code_, NULL);
  }
#endif
#define LNR_STRERROR_GEN(name, s) case LNR_##name : return s;
  switch (code_) {
  case LNR_OK:
    return "success";
  LNR_ERRNO_MAP(LNR_STRERROR_GEN)
  default:
    return "unknown error";
  }
#undef LNR_STRERROR_GEN
}

} // namespace linear
