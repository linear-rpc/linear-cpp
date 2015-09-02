#include "tv.h"

#include "linear/ssl_context.h"

#include "linear/log.h"

namespace linear {

class SSLContext::SSLContextImpl {
 public:
  explicit SSLContextImpl(const SSLContext::Method& method) {
    tv_ssl_library_init();
    switch (method) {
    case SSLContext::SSLv23_client:
      ssl_ctx_ = SSL_CTX_new(SSLv23_client_method());
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_TLSv1);
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv3);
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2);
      break;
    case SSLContext::SSLv23_server:
      ssl_ctx_ = SSL_CTX_new(SSLv23_server_method());
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_TLSv1);
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv3);
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2);
      break;
    case SSLContext::SSLv23:
      ssl_ctx_ = SSL_CTX_new(SSLv23_method());
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_TLSv1);
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv3);
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2);
      break;
    case SSLContext::TLSv1_1_client:
      ssl_ctx_ = SSL_CTX_new(TLSv1_1_client_method());
      break;
    case SSLContext::TLSv1_1_server:
      ssl_ctx_ = SSL_CTX_new(TLSv1_1_server_method());
      break;
    case SSLContext::TLSv1_1:
      ssl_ctx_ = SSL_CTX_new(TLSv1_1_method());
      break;
    default:
      ssl_ctx_ = SSL_CTX_new(TLSv1_1_method());
      break;
    }
  }
  ~SSLContextImpl() {
    SSL_CTX_free(ssl_ctx_);
  }

  bool SetCertificate(const std::string& certfile) {
    if (SSL_CTX_use_certificate_chain_file(ssl_ctx_, certfile.c_str()) != 1) {
      return false;
    }
    return true;
  }
  bool SetPrivateKey(const std::string& pkeyfile) {
    FILE* fp;

#ifdef _WIN32
    errno_t error;
    if ((error = fopen_s(&fp, pkeyfile.c_str(), "r")) != 0) {
      return false;
    }
#else
    if ((fp = fopen(pkeyfile.c_str(), "r")) == NULL) {
      return false;
    }
#endif

    EVP_PKEY* key;
    if ((key = PEM_read_PrivateKey(fp, NULL, NULL, NULL)) == NULL) {
      fclose(fp);
      return false;
    }

    if (SSL_CTX_use_PrivateKey(ssl_ctx_, key) != 1) {
      EVP_PKEY_free(key);
      fclose(fp);
      return false;
    }

    EVP_PKEY_free(key);
    fclose(fp);
    return true;
  }
  bool SetPrivateKey(const std::string& pkeyfile, const std::string& passphrase) {
    FILE* fp;

#ifdef _WIN32
    errno_t error;
    if ((error = fopen_s(&fp, pkeyfile.c_str(), "r")) != 0) {
      return false;
    }
#else
    if ((fp = fopen(pkeyfile.c_str(), "r")) == NULL) {
      return false;
    }
#endif

    EVP_PKEY* key;
    if ((key = PEM_read_PrivateKey(fp, NULL, NULL, reinterpret_cast<void*>(const_cast<char*>(passphrase.c_str())))) == NULL) {
      fclose(fp);
      return false;
    }

    if (SSL_CTX_use_PrivateKey(ssl_ctx_, key) != 1) {
      EVP_PKEY_free(key);
      fclose(fp);
      return false;
    }

    EVP_PKEY_free(key);
    fclose(fp);
    return true;
  }
  bool SetCiphers(const std::string& ciphers) {
    if (SSL_CTX_set_cipher_list(ssl_ctx_, ciphers.c_str()) != 1) {
      return false;
    }
    return true;
  }
  bool SetCAFile(const std::string& cafile) {
    cafile_ = cafile;
    const char* caf = cafile_.empty() ? NULL : cafile_.c_str();
    const char* cap = capath_.empty() ? NULL : capath_.c_str();
    if (SSL_CTX_load_verify_locations(ssl_ctx_, caf, cap) != 1) {
      return false;
    }
    return true;
  }
  bool SetCAPath(const std::string& capath) {
    capath_ = capath;
    const char* caf = cafile_.empty() ? NULL : cafile_.c_str();
    const char* cap = capath_.empty() ? NULL : capath_.c_str();
    if (SSL_CTX_load_verify_locations(ssl_ctx_, caf, cap) != 1) {
      return false;
    }
    return true;
  }
  void SetVerifyMode(const SSLContext::VerifyMode& mode, int (*verify_callback)(int, X509_STORE_CTX*)) {
    switch (mode) {
    case SSLContext::VERIFY_FAIL_IF_NO_PEER_CERT:
      SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);
      break;
    case SSLContext::VERIFY_PEER:
      SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE, verify_callback);
      break;
    case SSLContext::VERIFY_NONE:
    default:
      SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_NONE, NULL);
      break;
    }
  }
  SSL_CTX* GetHandle() const {
    return ssl_ctx_;
  }

 private:
  SSL_CTX* ssl_ctx_;
  std::string cafile_;
  std::string capath_;
};

SSLContext::SSLContext()
  : pimpl_(new SSLContext::SSLContextImpl(SSLContext::SSLv23)) {
}
SSLContext::SSLContext(const SSLContext::Method& method)
  : pimpl_(new SSLContext::SSLContextImpl(method)) {
}
SSLContext::SSLContext(const SSLContext& obj) : pimpl_(obj.pimpl_) {
}
SSLContext& SSLContext::operator=(const SSLContext& obj) {
  pimpl_ = obj.pimpl_;
  return *this;
}
SSLContext::~SSLContext() {
}

bool SSLContext::SetCertificate(const std::string& certfile) {
  return pimpl_->SetCertificate(certfile);
}
bool SSLContext::SetPrivateKey(const std::string& pkeyfile) {
  return pimpl_->SetPrivateKey(pkeyfile);
}
bool SSLContext::SetPrivateKey(const std::string& pkeyfile, const std::string& passphrase) {
  return pimpl_->SetPrivateKey(pkeyfile, passphrase);
}
bool SSLContext::SetCiphers(const std::string& ciphers) {
  return pimpl_->SetCiphers(ciphers);
}
bool SSLContext::SetCAFile(const std::string& cafile) {
  return pimpl_->SetCAFile(cafile);
}
bool SSLContext::SetCAPath(const std::string& capath) {
  return pimpl_->SetCAPath(capath);
}
void SSLContext::SetVerifyMode(const SSLContext::VerifyMode& mode, int (*verify_callback)(int, X509_STORE_CTX*)) {
  pimpl_->SetVerifyMode(mode, verify_callback);
}
SSL_CTX* SSLContext::GetHandle() const {
  return pimpl_->GetHandle();
}

}  // namespace linear
