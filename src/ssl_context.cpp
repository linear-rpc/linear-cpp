#include <fstream>

#include "tv.h"

#include "linear/ssl_context.h"

namespace linear {

class SSLContext::SSLContextImpl {
 public:
  explicit SSLContextImpl(const SSLContext::Method& method) {
    tv_ssl_library_init();
    switch (method) {
    case SSLContext::SSLv23_client:
      ssl_ctx_ = SSL_CTX_new(SSLv23_client_method());
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1);
      break;
    case SSLContext::SSLv23_server:
      ssl_ctx_ = SSL_CTX_new(SSLv23_server_method());
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1);
      break;
    case SSLContext::SSLv23:
      ssl_ctx_ = SSL_CTX_new(SSLv23_method());
      SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1);
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
    SSL_CTX_set_default_verify_paths(ssl_ctx_);
  }
  ~SSLContextImpl() {
    SSL_CTX_free(ssl_ctx_);
  }
  bool SetCertificate(const std::string& file,
                      SSLContext::Encoding encoding) {
    if (encoding == SSLContext::PEM) {
      return (SSL_CTX_use_certificate_chain_file(ssl_ctx_, file.c_str()) == 1);
    } else if (encoding == SSLContext::DER) {
      size_t siz = getFileSize(file);
      if (siz == 0) {
        return false;
      }
      std::ifstream ifs(file.c_str(), std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        return false;
      }
      char* data = NULL;
      try {
        data = new char[siz];
      } catch(...) {
        return false;
      }
      ifs.read(data, siz);
      if (static_cast<size_t>(ifs.gcount()) != siz) {
        delete[] data;
        return false;
      }
      BIO* bio = BIO_new(BIO_s_mem());
      if (bio == NULL) {
        delete[] data;
        return false;
      }
      if (BIO_write(bio, data, siz) == -1) {
        BIO_free_all(bio);
        delete[] data;
        return false;
      }
      delete[] data;
      X509* cert = d2i_X509_bio(bio, NULL);
      if (cert == NULL) {
        BIO_free_all(bio);
        return false;
      }
      BIO_free_all(bio);
      if (SSL_CTX_use_certificate(ssl_ctx_, cert) != 1) {
        X509_free(cert);
        return false;
      }
      X509_free(cert);
      return true;
    }
    return false;
  }
  // DER can't have passphrase, so ignore passphrase var arg.
  bool SetPrivateKey(const std::string& file, const std::string& passphrase,
                     SSLContext::Encoding encoding) {
    EVP_PKEY* key = NULL;
    if (encoding == SSLContext::PEM) {
      const char* p = (passphrase.size() > 0) ? passphrase.c_str() : NULL;
      FILE* fp = NULL;

#ifdef _WIN32
      errno_t error;
      if ((error = fopen_s(&fp, file.c_str(), "r")) != 0) {
        return false;
      }
#else
      if ((fp = fopen(file.c_str(), "r")) == NULL) {
        return false;
      }
#endif

      key = PEM_read_PrivateKey(fp, NULL, NULL, const_cast<char*>(p));
      fclose(fp);
    } else if (encoding == SSLContext::DER) {
      size_t siz = getFileSize(file);
      if (siz == 0) {
        return false;
      }
      std::ifstream ifs(file.c_str(), std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        return false;
      }
      char* data = NULL;
      try {
        data = new char[siz];
      } catch(...) {
        return false;
      }
      ifs.read(data, siz);
      if (static_cast<size_t>(ifs.gcount()) != siz) {
        delete[] data;
        return false;
      }
      BIO* bio = BIO_new(BIO_s_mem());
      if (bio == NULL) {
        delete[] data;
        return false;
      }
      if (BIO_write(bio, data, siz) == -1) {
        BIO_free_all(bio);
        delete[] data;
        return false;
      }
      key = d2i_PrivateKey_bio(bio, NULL);
      BIO_free_all(bio);
      delete[] data;
    }
    if (key == NULL) {
      return false;
    }
    if (SSL_CTX_use_PrivateKey(ssl_ctx_, key) != 1) {
      EVP_PKEY_free(key);
      return false;
    }
    EVP_PKEY_free(key);
    return true;
  }
  bool SetCiphers(const std::string& ciphers) {
    return (SSL_CTX_set_cipher_list(ssl_ctx_, ciphers.c_str()) == 1);
  }
  bool SetCAFile(const std::string& file, SSLContext::Encoding encoding) {
    if (encoding == SSLContext::PEM) {
      return (SSL_CTX_load_verify_locations(ssl_ctx_, file.c_str(), NULL) == 1);
    } else if (encoding == SSLContext::DER) {
      size_t siz = getFileSize(file);
      if (siz == 0) {
        return false;
      }
      std::ifstream ifs(file.c_str(), std::ios::in | std::ios::binary);
      if (ifs.fail()) {
        return false;
      }
      char* data = NULL;
      try {
        data = new char[siz];
      } catch(...) {
        return false;
      }
      ifs.read(data, siz);
      if (static_cast<size_t>(ifs.gcount()) != siz) {
        delete[] data;
        return false;
      }
      X509_STORE* store = SSL_CTX_get_cert_store(ssl_ctx_);
      if (store == NULL) {
        delete[] data;
        return false;
      }
      const unsigned char* pdata = reinterpret_cast<unsigned char*>(data);
      X509* cert = d2i_X509(NULL, &pdata, siz);
      if (cert == NULL) {
        delete[] data;
        return false;
      }
      if (X509_STORE_add_cert(store, cert) == 0) {
        X509_free(cert);
        return false;
      }
      X509_free(cert);
      delete[] data;
      return true;
    }
    return false;
  }
  bool SetCAPath(const std::string& path) {
    return (SSL_CTX_load_verify_locations(ssl_ctx_, NULL, path.c_str()) == 1);
  }
  void SetVerifyMode(const SSLContext::VerifyMode& mode,
                     int (*verify_callback)(int, X509_STORE_CTX*)) {
    switch (mode) {
    case SSLContext::VERIFY_FAIL_IF_NO_PEER_CERT:
      SSL_CTX_set_verify(ssl_ctx_,
                         SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                         verify_callback);
      break;
    case SSLContext::VERIFY_PEER:
      SSL_CTX_set_verify(ssl_ctx_,
                         SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,
                         verify_callback);
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
  size_t getFileSize(const std::string& file) {
    std::ifstream ifs(file.c_str(), std::ios::in | std::ios::binary);
    return (ifs.fail() ? 0 : static_cast<size_t>(ifs.seekg(0, std::ios::end).tellg()));
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

bool SSLContext::SetCertificate(const std::string& file,
                                linear::SSLContext::Encoding encoding) {
  return pimpl_->SetCertificate(file, encoding);
}
bool SSLContext::SetPrivateKey(const std::string& file, const std::string& passphrase,
                               linear::SSLContext::Encoding encoding) {
  return pimpl_->SetPrivateKey(file, passphrase, encoding);
}
bool SSLContext::SetCiphers(const std::string& ciphers) {
  return pimpl_->SetCiphers(ciphers);
}
bool SSLContext::SetCAFile(const std::string& file,
                           linear::SSLContext::Encoding encoding) {
  return pimpl_->SetCAFile(file, encoding);
}
bool SSLContext::SetCAPath(const std::string& path) {
  return pimpl_->SetCAPath(path);
}
void SSLContext::SetVerifyMode(const SSLContext::VerifyMode& mode,
                               int (*verify_callback)(int, X509_STORE_CTX*)) {
  pimpl_->SetVerifyMode(mode, verify_callback);
}
SSL_CTX* SSLContext::GetHandle() const {
  return pimpl_->GetHandle();
}

}  // namespace linear
