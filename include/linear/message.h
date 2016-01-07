/**
 * @file message.h
 * classes, macros, apis and typedefs for Request, Response, Notify
 */

#ifndef LINEAR_MESSAGE_H_
#define LINEAR_MESSAGE_H_

// TODO: HAVE_LIMITS_H, HAVE_STDINT_H
#include <limits.h>
#include <stdint.h>

#include "linear/any.h"
#include "linear/socket.h"

#define LINEAR_PACK(...) MSGPACK_DEFINE(__VA_ARGS__)

namespace linear {

/**
 * @typedef linear::message_type_t
 * message type
 * <ul>
 * <li>linear::REQUEST</li>
 * <li>linear::RESPONSE</li>
 * <li>linear::NOTIFY</li>
 * </ul>
 */
typedef uint8_t message_type_t;

/// @cod hidden
static const message_type_t REQUEST   = 0;
static const message_type_t RESPONSE  = 1;
static const message_type_t NOTIFY    = 2;
static const message_type_t UNDEFINED = 3;
/// @endcond

/**
 * @class Message message.h "linear/message.h"
 * Super class of several concrete messages
 */
class LINEAR_EXTERN Message {
 public:
  /// @cond hidden
  Message() : type(linear::UNDEFINED) {}
  explicit Message(linear::message_type_t t) : type(t) {}
  virtual ~Message() {}
  /// @endcond

  /**
   * downcast method to get concrete messages
   * @see linear::Handler.OnMessage, linear::Handler.OnError
   */
  template <typename Value>
  inline Value as() const {
    return dynamic_cast<Value&>(const_cast<linear::Message&>(*this));
  }

 public:
  /**
   * message type
   * @see linear::message_type_t
   */
  linear::message_type_t type;

  /// @cond hidden
  MSGPACK_DEFINE(type);
  /// @endcond
};

class Response;

/**
 * @class Request message.h "linear/message.h"
 * A Request object
 * @see linear::Handler.OnMessage
 */
class LINEAR_EXTERN Request : public linear::Message {
 public:
  /// @cond hidden
  Request();
  /// @endcond

  /**
   * Request Constructor
   * @param m method name
   * @param p parameter
   * @note
   * If parameter type is not primitive, the parameter is need to be include LINEAR_PACK
   *
   @code
   struct Foo {
    Foo() : i(1), s("param") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   // method = "int", params = <int>(1)
   linear::Request request_int("int", 1);

   // method = "string", params = <string>("param")
   linear::Request request_string("string", std::string("param"));

   // method = "foo", params = [<int>(1), <string>("param")]
   linear::Request request_foo("foo", Foo());
   @endcode
   */
  Request(const std::string& m, const linear::type::any& p);

  /**
   * send request to peer node with timeout == 30sec
   * @param socket a linear::Socket object
   * @note we may change to use Socket.Send mainly
   *
   @code
   struct Foo {
    Foo() : i(1), s("param") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   linear::TCPClient client(handler);
   linear::Socket socket = client.CreateSocket("host", port);
   linear::Request request_foo("foo", Foo());
   uint32_t message_id = request_foo.msgid;
   request_foo.Send(socket);
   std::cout << "send request: id == " << message_id << std::endl;
   @endcode
   */
  linear::Error Send(const linear::Socket& socket) const;
  /**
   * send request to peer node with timeout
   * @param socket a linear::Socket object
   * @param timeout request timeout (msec)
   * @note we may change to use Socket.Send mainly
   */
  linear::Error Send(const linear::Socket& socket, int timeout) const;
  /**
   * send request to peer node with timeout and recv OnResponse callback by Closure
   * (ignore all of error)
   * @param socket a linear::Socket object
   * @param timeout request timeout (msec)
   * @param on_response OnResponse Callback function
   * @note EXPERIMENTAL: we may move this method into Socket.Send
   */
  template <typename ResponseCallbackType>
  linear::Error Send(const linear::Socket& socket, int timeout, ResponseCallbackType& on_response);
  /**
   * send request to peer node with timeout and recv OnResponse and OnError callback by Closure
   * @param socket a linear::Socket object
   * @param timeout request timeout (msec)
   * @param on_response OnResponse Callback function
   * @param on_error OnError Callback function
   * @note EXPERIMENTAL: we may move this method into Socket.Send
   */
  template <typename ResponseCallbackType, typename ErrorCallbackType>
  linear::Error Send(const linear::Socket& socket, int timeout, ResponseCallbackType& on_response, ErrorCallbackType& on_error);

  /// @cond hidden
  bool HasResponseCallback() const;
  bool HasErrorCallback() const;
  void FireResponseCallback(const linear::Socket& socket, const linear::Response& response) const;
  void FireErrorCallback(const linear::Socket& socket, const linear::Request& request, const linear::Error& error) const;
  /// @endcond

 private:
  class IResponseCallbackHolder;
  class IErrorCallbackHolder;

  template <typename CallbackType>
  class ResponseCallbackHolder;
  template <typename CallbackType>
  class ErrorCallbackHolder;

  linear::shared_ptr<IResponseCallbackHolder> on_response_holder_;
  linear::shared_ptr<IErrorCallbackHolder> on_error_holder_;

 public:
  /**
   * message id\n
   * you can use this id as request id for checking after calling Send method
   */
  uint32_t msgid;
  /**
   * request method string
   */
  std::string method;
  /**
   * request params
   * @see linear::type::any
   */
  linear::type::any params;

  /// @cond hidden
  /**
   * timeout time(ms)
   */
  int timeout_;

  MSGPACK_DEFINE(type, msgid, method, params);
  /// @endcond
};

/**
 * @class Response message.h "linear/message.h"
 * A Response object
 * @see linear::Handler.OnMessage
 */
class LINEAR_EXTERN Response : public linear::Message {
 public:
  /// @cond hidden
  Response() : Message(linear::RESPONSE), msgid(0) {}
  Response(uint32_t id, const linear::type::any& r, const linear::type::any& e, const linear::Request& req)
    : Message(linear::RESPONSE), request(req), msgid(id), result(r), error(e) {}
  /// @endcond

  /**
   * Valid Response Constructer
   * @param id msgid of request\n
   * you must fill this id from your received request.
   * @param r result
   * @note
   * If result type is not primitive, the parameter is need to be include LINEAR_PACK
   *
   @code
   struct Foo {
    Foo() : i(1), s("result") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   // result = <int>(1), error = nil
   linear::Response response_int(request.msgid, 1);

   // result = <string>("result"), error = nil
   linear::Response response_string(request.msgid, std::string("result"));

   // result = [<int>(1), <string>("result")], error = nil
   linear::Response response_foo(request.msgid, Foo());
   @endcode
   */
  Response(uint32_t id, const linear::type::any& r) : Message(linear::RESPONSE), msgid(id), result(r) {}
  /**
   * Error Response Constructor
   * @param id msgid of request\n
   * you must fill this id from your received request.
   * @param r result must be linear::type::nil()
   * @param e error
   * @note
   * If error type is not primitive, the parameter is need to be include LINEAR_PACK
   *
   @code
   struct Foo {
    Foo() : i(-1), s("error") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   // result = nil, error = <int>(-1)
   linear::Response err_response_int(request.msgid, linear::type::nil(), -1);

   // result = nil, error = <string>("error")
   linear::Response err_response_string(request.msgid, linear::type::nil(), std::string("error"));

   // result = nil, error = [<int>(-1), <string>("error")]
   linear::Response err_response_foo(request.msgid, linear::type::nil(), Foo());
   @endcode
   */
  Response(uint32_t id, const linear::type::any& r, const linear::type::any& e)
    : Message(linear::RESPONSE), msgid(id), result(r), error(e) {}
  /**
   * send response to peer node
   * @param socket linear::Socket object
   * @note we may change to use Socket.Send mainly
   *
   @code
   void YourHandler::OnMessage(const linear::Socket& socket, const linear::Message& message) {
     switch(message.type) {
     case linear::REQUEST:
       {
         linear::Request request = message.as<linear::Request>();
         linear::Response response(request.msgid, std::string("result"));
         response.Send(socket);
         break;
       }
     }
   }
   @endcode
   */
  linear::Error Send(const linear::Socket& socket) const;

 public:
  /**
   * reference of original Request
   */
  linear::Request request;
  /**
   * message id\n
   * points to which Request
   */
  uint32_t msgid;
  /**
   * result
   * @see linear::type::any
   */
  linear::type::any result;
  /**
   * error
   * @see linear::type::any
   */
  linear::type::any error;

  /// @cond hidden
  MSGPACK_DEFINE(type, msgid, error, result);
  /// @endcond
};

/**
 * @class Notify message.h "linear/message.h"
 * A Notify object
 * @see linear::Handler.OnMessage
 */
class LINEAR_EXTERN Notify : public linear::Message {
 public:
  /// @cond hidden
  Notify() : Message(linear::NOTIFY) {}
  /// @endcond

  /**
   * Notify Constructor
   * @param m method name
   * @param p parameter
   * @note
   * If parameter type is not primitive, the parameter is need to be include LINEAR_PACK
   *
   @code
   struct Foo {
    Foo() : i(1), s("params") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   // method = "int", params = <int>(1)
   linear::Notify notify_int("int", 1);

   // method = "string", params = <string>("params")
   linear::Notify notify_string("string", std::string("param"));

   // method = "foo", params = [<int>(1), <string>("params")]
   linear::Notify notify_foo("foo", Foo());
   @endcode
   */
  Notify(const std::string& m, const linear::type::any& p) : Message(linear::NOTIFY), method(m), params(p) {}
  /**
   * send notify to peer node
   * @param socket linear::Socket object
   * @note we may change to use Socket.Send mainly
   *
   @code
   struct Foo {
    Foo() : i(-1), s("error") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   linear::TCPClient client(handler);
   linear::Socket socket = client.CreateSocket("host", port);
   linear::Notify notify_foo("foo", Foo());
   notify_foo.Send(socket);
   std::cout << "sended notify" << std::endl;
   @endcode
   */
  linear::Error Send(const linear::Socket& socket) const;
  /**
   * send message to group
   * @param group_name socket group
   * @see linear::Group
   *
   @code
   struct Foo {
    Foo() : i(-1), s("error") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   linear::TCPClient client(handler);
   linear::Notify notify_foo("foo", Foo());
   notify_foo.Send("group_name");
   std::cout << "sended notify to group_name" << std::endl;
   @endcode
   */
  void Send(const std::string& group_name) const;
  /**
   * send message to group except for specified socket
   * @param group_name socket group
   * @param except_socket excepting socket
   * @see linear::Group
   *
   @code
   struct Foo {
    Foo() : i(-1), s("error") {}
    ~Foo() {}

    int i;
    std::string s;

    LINEAR_PACK(i, s);
   };

   linear::TCPClient client(handler);
   linear::Socket socket1 = client.CreateSocket("server-1", port);
   socket1.Connect();
   linear::Socket socket2 = client.CreateSocket("server-2", port);
   socket1.Connect();

   linear::Group::Join("group_name", socket1);
   linear::Group::Join("group_name", socket2);

   linear::Notify notify_foo("foo", Foo());

   // not send server-2!
   notify_foo.Send("group_name", socket2);
   std::cout << "sended notify to group_name except for socket.id == "
             << socket2.GetId() << std::endl;
   @endcode
   */
  void Send(const std::string& group_name, const linear::Socket& except_socket) const;

 public:
  /**
   * notify method string
   */
  std::string method;
  /**
   * notify params
   * @see linear::type::any
   */
  linear::type::any params;

  /// @cond hidden
  MSGPACK_DEFINE(type, method, params);
  /// @endcond
};

}  // namespace linear

#include "linear/private/message_priv.h"
#endif  // LINEAR_MESSAGE_H_
