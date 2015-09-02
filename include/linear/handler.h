/**
 * @file handler.h
 * Super class definition for several concrete handler classes
 **/

#ifndef LINEAR_HANDLER_H_
#define LINEAR_HANDLER_H_

#include "linear/message.h"
#include "linear/private/observer.h"

namespace linear {

class Socket;

/**
 * @class Handler handler.h "linaer/handler.h"
 * Super class for several callback functions.
 * Application developers need to implement a concrete class inheriting this class.
 */
class LINEAR_EXTERN Handler {
 public:
  /// @cond hidden
  Handler();
  virtual ~Handler();
  shared_ptr<linear::Observer<linear::Handler> > GetObserver() const;
  /// @endcond

  /**
   * Callback function called when socket is connected
   * @param socket connected socket
   *
   @code
   void YourHandler::OnConnect(const linear::Socket& socket) {
     std::cerr << "connected" << std::endl;
   }
   @endcode
   */
  virtual void OnConnect(const linear::Socket& socket) {}
  /**
   * Callback function called when socket is disconnected
   * @param socket disconnected socket
   * @param error linear::Error for disconnect reason
   *
   @code
   void YourHandler::OnDisconnect(const linear::Socket& socket, const linear::Error& error) {
     std::cerr << "disconnected" << std::endl;
   }
   @endcode
   */
  virtual void OnDisconnect(const linear::Socket& socket, const linear::Error& error) {}
  /**
   * Callback function called when received message from peer
   * @param socket connected socket
   * @param message linear::Message object ref.
   * You can get real message to check linear::message_type_t and use linear::Message.as method
   *
   @code
   void YourHandler::OnMessage(const linear::Socket& socket, const linear::Message& message) {
     switch(message.type) {
     case linear::REQUEST:
       {
         linear::Request request = message.as<linear::Request>();
         std::cout << "recv request: msgid = " << request.msgid
                   << ", method = " << request.method
                   << ", params = " << request.params.stringify() << std::endl;
       }
       break;
     case linear::RESPONSE:
       {
         linear::Response response = message.as<linear::Response>();
         std::cout << "recv response: msgid = " << response.msgid
                   << ", result = " << response.result.stringify()
                   << ", error = " << response.error.stringify() << std::endl;
       }
       break;
     case linear::NOTIFY:
       {
         linear::Notify notify = message.as<linear::Notify>();
         std::cout << "recv notify: "
                   << "method = " << notify.method
                   << ", params = " << notify.params.stringify() << std::endl;
       }
       break;
     default:
       {
         std::cerr << "BUG: plz inform to linear-developpers" << std::endl;
         assert(false);
       }
       break;
     }
   }
   @endcode
   */
  virtual void OnMessage(const linear::Socket& socket, const linear::Message& message) {}
  /**
   * Callback function called when occuring some error for your message sended
   * @param socket connected socket
   * @param message linear::Message object ref.
   * You can get real message to check linear::message_type_t and use linear::Message.as method
   * @param error linear::Error for error reason
   *
   @code
   void YourHandler::OnError(const linear::Socket& socket, const linear::Message& message, const linear::Error& error) {
     switch(message.type) {
     case linear::REQUEST:
       {
         linear::Request request = message.as<linear::Request>();
         std::cerr << "error to send request: msgid = " << request.msgid
                   << ", method = " << request.method
                   << ", params = " << request.params.stringify()
                   << ", error = " << error.Message() << std::endl;
       }
       break;
     case linear::RESPONSE:
       {
         linear::Response response = message.as<linear::Response>();
         std::cerr << "error to send response: msgid = " << response.msgid
                   << ", result = " << response.result.stringify()
                   << ", error = " << response.error.stringify()
                   << ", error = " << error.Message() << std::endl;
       }
       break;
     case linear::NOTIFY:
       {
         linear::Notify notify = message.as<linear::Notify>();
         std::cerr << "error to send notify: "
                   << "method = " << notify.method
                   << ", params = " << notify.params.stringify()
                   << ", error = " << error.Message() << std::endl;
       }
       break;
     default:
       {
         std::cerr << "BUG: plz inform to linear-developpers" << std::endl;
         assert(false);
       }
       break;
     }
   }
   @endcode
   */
  virtual void OnError(const linear::Socket& socket, const linear::Message& message, const linear::Error& error) {}

 private:
  shared_ptr<linear::Observer<linear::Handler> > observer_;
};

}  // namespace linear

#endif  // LINEAR_HANDLER_H_
