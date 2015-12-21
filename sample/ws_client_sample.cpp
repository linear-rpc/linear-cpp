// a sample for WSClient

#if !defined _WIN32
# include <unistd.h> // for getopt
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "linear/ws_client.h"
#include "linear/log.h"
#include "linear/timer.h"

// You can Send-Recv some structure like as follows
struct Base {
  Base() : int_val(1234), double_val(3.14), string_val("Client") {
    vector_val.push_back(1);
    map_val.insert(std::make_pair("key", 1));
  }
  ~Base() {}
  int int_val;
  double double_val;
  std::string string_val;
  std::vector<int> vector_val;
  std::map<std::string, int> map_val;

  // magic(same as MSGPACK_DEFINE)
  LINEAR_PACK(int_val, double_val, string_val, vector_val, map_val);
};
struct Derived : public Base {
  Derived() : Base(), derived_val(0) {}
  ~Derived() {}

  int derived_val;

  // magic(same as MSGPACK_DEFINE)
  LINEAR_PACK(int_val, double_val, string_val, vector_val, map_val, // Base::
              derived_val);
};

// You need to implement concrete handler class like as follows.
class ApplicationHandler : public linear::Handler {
 public:
  static void Reconnect(void* args) {
    linear::Socket* socket = reinterpret_cast<linear::Socket*>(args);
    socket->Connect();
    delete socket;
  }

 public:
  ApplicationHandler() : num_of_retry_(3) {}
  ~ApplicationHandler() {}
  void SetNumOfRetry(int retry) {
    num_of_retry_ = retry;
  }
  void OnConnect(const linear::Socket& socket) {
    const linear::Addrinfo& info = socket.GetPeerInfo();
    std::cout << "OnConnect: " << info.addr << ":" << info.port << std::endl;

    // WSSocket specific
    linear::WSResponseContext context = socket.as<linear::WSSocket>().GetWSResponseContext();
    std::cout << "--- Response Headers ---" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = context.headers.begin();
         it != context.headers.end(); it++) {
      std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "--- Headers End ---" << std::endl;
    // WSSocket specific end
  }
  void OnDisconnect(const linear::Socket& socket, const linear::Error& err) {

    // WSSocket specific
    linear::WSResponseContext response_context = socket.as<linear::WSSocket>().GetWSResponseContext();
    std::cout << "Response Code: " << response_context.code << std::endl;
    std::cout << "--- Response Headers ---" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = response_context.headers.begin();
         it != response_context.headers.end(); it++) {
      std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "--- Headers End ---" << std::endl;

    static int num_of_try = 0;
    if (num_of_try < num_of_retry_) {
      // retry to connect after 1 sec
      timer_.Start(ApplicationHandler::Reconnect, 1000, new linear::Socket(socket));
      num_of_try++;
    } else {
      // give up to connect
      const linear::Addrinfo& info = socket.GetPeerInfo();
      std::cout << "OnDisconnect: " << info.addr << ":" << info.port << std::endl;
      num_of_try = 0;
    }
  }
  void OnMessage(const linear::Socket& socket, const linear::Message& msg) {
    const linear::Addrinfo& info = socket.GetPeerInfo();
    switch(msg.type) {
    case linear::REQUEST:
      {
        linear::Request request = msg.as<linear::Request>();
        std::cout << "recv Request: msgid = " << request.msgid
                  << ", method = \"" << request.method << "\""
                  << ", params = " << request.params.stringify()
                  << " from " << info.addr << ":" << info.port << std::endl;
        linear::Response response(request.msgid, linear::type::nil(), std::string("This client does not handle request"));
        response.Send(socket);
      }
      break;
    case linear::RESPONSE:
      {
        linear::Response response = msg.as<linear::Response>();
        std::cout << "recv Response(Handler): msgid = " << response.msgid
                  << ", result = " << response.result.stringify()
                  << ", error = " << response.error.stringify()
                  << " from " << info.addr << ":" << info.port << std::endl;
        std::cout << "origin request: msgid = " << response.request.msgid
                  << ", method = \"" << response.request.method << "\""
                  << ", params = " << response.request.params.stringify() << std::endl;
      }
      break;
    case linear::NOTIFY:
      {
        linear::Notify notify = msg.as<linear::Notify>();
        std::cout << "recv Notify: "
                  << "method = \"" << notify.method << "\""
                  << ", params = " << notify.params.stringify()
                  << " from " << info.addr << ":" << info.port << std::endl;
        try {
          Derived data = notify.params.as<Derived>();
          std::cout << "parameters detail" << std::endl;
          std::cout << "Base::"
                    << "int: " << data.int_val
                    << ", double: " << data.double_val
                    << ", string: " << data.string_val
                    << ", vector: " << data.vector_val[0]
                    << ", map: {\"key\": " << data.map_val["key"] << "}" << std::endl;
          std::cout << "Derived::int: " << data.derived_val << std::endl;
        } catch(const std::bad_cast&) {
          std::cout << "invalid type cast" << std::endl;
        }
      }
      break;
    default:
      {
        std::cout << "BUG: plz inform to linear-developpers" << std::endl;
      }
      break;
    }
  }
  void OnError(const linear::Socket& socket, const linear::Message& msg, const linear::Error& err) {
    switch(msg.type) {
    case linear::REQUEST:
      {
        linear::Request request = msg.as<linear::Request>();
        std::cout << "Error to Send Request: msgid = " << request.msgid
                  << ", method = \"" << request.method << "\""
                  << ", params = " << request.params.stringify()
                  << ", err = " << err.Message() << std::endl;
      }
      break;
    case linear::RESPONSE:
      {
        linear::Response response = msg.as<linear::Response>();
        std::cout << "Error to Send Response: msgid = " << response.msgid
                  << ", result = " << response.result.stringify()
                  << ", error = " << response.error.stringify()
                  << ", err = " << err.Message() << std::endl;
        std::cout << "origin request: msgid = " << response.request.msgid
                  << ", method = \"" << response.request.method << "\""
                  << ", params = " << response.request.params.stringify() << std::endl;
      }
      break;
    case linear::NOTIFY:
      {
        linear::Notify notify = msg.as<linear::Notify>();
        std::cout << "Error to Send Notify: "
                  << "method = \"" << notify.method << "\""
                  << ", params = " << notify.params.stringify()
                  << ", err = " << err.Message() << std::endl;
      }
      break;
    default:
      {
        std::cout << "BUG: plz inform to linear-developpers" << std::endl;
      }
      break;
    }
  }

private:
  int num_of_retry_;
  linear::Timer timer_;
};

void usage(char* name) {
  std::cout << "Usage: " << std::string(name) << " [Host := 127.0.0.1] [Port := 37800]" << std::endl;
}

int main(int argc, char* argv[]) {

#if _WIN32
  linear::log::SetLevel(linear::log::LOG_DEBUG);
  linear::log::EnableStderr();
  std::string host = (argc > 1) ? std::string(argv[1]) : "127.0.0.1";
  int port = (argc > 2) ? atoi(argv[2]) : 37800;
#else
  bool show_log = false;
  linear::log::Level level = linear::log::LOG_OFF;
  int ch, lv;
  extern char* optarg;
  extern int optind;

  while ((ch = getopt(argc, argv, "hl:")) != -1) {
    switch(ch) {
    case 'h':
      usage(argv[0]);
      return 0;
    case 'l':
      show_log = true;
      lv = atoi(optarg);
      lv = (lv < 0) ? 0 : ((lv > 4) ? 4 : lv);
      level = static_cast<linear::log::Level>(lv);
      break;
    default:
      break;
    }
  }
  argc -= optind;
  argv += optind;

  if (show_log) {
    linear::log::SetLevel(level);
    linear::log::EnableStderr();
  }
  std::string host = (argc >= 1) ? std::string(argv[0]) : "127.0.0.1";
  int port = (argc >= 2) ? atoi(argv[1]) : 37800;
#endif

  linear::shared_ptr<ApplicationHandler> handler = linear::shared_ptr<ApplicationHandler>(new ApplicationHandler());
  linear::WSClient client = linear::WSClient(handler);
  linear::WSRequestContext context;
  context.path = "linear";
  context.query = "?foo=bar&alice=bob";
  linear::WSSocket socket = client.CreateSocket(host, port, context);

  std::string cmd;
  std::cout << "Commands: connect, disconnect, echo, notify, exit" << std::endl;
  std::getline(std::cin, cmd);
  while (true) {
    if (cmd == "connect") {
      handler->SetNumOfRetry(3);
      linear::Error err = socket.Connect();
      if (err.Code() != linear::LNR_OK) {
        std::cout << err.Message() << std::endl;
      }
    } else if (cmd == "disconnect") {
      handler->SetNumOfRetry(0);
      linear::Error err = socket.Disconnect();
      if (err.Code() != linear::LNR_OK) {
        std::cout << err.Message() << std::endl;
      }
    } else if (cmd == "echo") {
      std::string data;
      std::cout << "A Response will be received by ApplicationHandler::OnMessage asynchronously" << std::endl;
      std::cout << "Input some words: " << std::endl;
      std::getline(std::cin, data);

      linear::Request req(cmd, data);
      std::cout << "msgid: " << req.msgid << std::endl;
      linear::Error err = req.Send(socket, 3000); // timeout == 3 sec
      if (err.Code() != linear::LNR_OK) {
        std::cout << err.Message() << std::endl;
      }
    } else if (cmd == "notify") {
      linear::Notify notify("from client", Derived());
      linear::Error err = notify.Send(socket);
      if (err.Code() != linear::LNR_OK) {
        std::cout << err.Message() << std::endl;
      }
    } else if (cmd == "exit") {
      break;
    }
    std::getline(std::cin, cmd);
  }
  return 0;
}
