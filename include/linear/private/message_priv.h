/**
 * @file message_priv.h
 * Implementations of Message class templates
 */

#ifndef LINEAR_PRIVATE_MESSAGE_PRIV_H_
#define LINEAR_PRIVATE_MESSAGE_PRIV_H_

namespace linear {

class Request::IResponseCallbackHolder {
 public:
  virtual ~IResponseCallbackHolder() {}
  virtual void Fire(const linear::Socket& socket, const linear::Response& response) const = 0;
};

template <typename CallbackType>
class Request::ResponseCallbackHolder : public Request::IResponseCallbackHolder {
 public:
  ResponseCallbackHolder(CallbackType& callback) : callback_(callback) {}
  virtual ~ResponseCallbackHolder() {}

  void Fire(const linear::Socket& socket, const linear::Response& response) const {
    callback_(socket, response);
  }

 private:
  CallbackType& callback_;
};

class Request::IErrorCallbackHolder {
 public:
  virtual ~IErrorCallbackHolder() {}
  virtual void Fire(const linear::Socket& socket, const linear::Request& request, const linear::Error& error) const = 0;
};

template <typename CallbackType>
class Request::ErrorCallbackHolder : public Request::IErrorCallbackHolder {
 public:
  ErrorCallbackHolder(CallbackType& callback) : callback_(callback) {}
  virtual ~ErrorCallbackHolder() {}

  void Fire(const linear::Socket& socket, const linear::Request& request, const linear::Error& error) const {
    callback_(socket, request, error);
  }

 private:
  CallbackType& callback_;
};

template <typename ResponseCallbackType>
linear::Error Request::Send(const linear::Socket& socket, int timeout, ResponseCallbackType& on_response) {
  on_response_holder_ = linear::shared_ptr<IResponseCallbackHolder>(new ResponseCallbackHolder<ResponseCallbackType>(on_response));
  return Send(socket, timeout);
}

template <typename ResponseCallbackType, typename ErrorCallbackType>
linear::Error Request::Send(const linear::Socket& socket, int timeout, ResponseCallbackType& on_response, ErrorCallbackType& on_error) {
  on_error_holder_ = linear::shared_ptr<IErrorCallbackHolder>(new ErrorCallbackHolder<ErrorCallbackType>(on_error));
  return Send(socket, timeout, on_response);
}

}  // namespace linear

#endif  // LINEAR_PRIVATE_MESSAGE_PRIV_H_
