// a sample for SSLServer

#if !defined _WIN32
# include <unistd.h> // for getopt
#endif

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "linear/ssl_server.h"
#include "linear/log.h"
#include "linear/group.h"

#define SERVER_CERT        "./certs/server.pem"
#define SERVER_PRIVATE_KEY "./certs/server.key"
#define CA_CERT            "./certs/ca.pem"

// You can Send-Recv some structure like as follows
struct Base {
  Base() : int_val(-1234), double_val(-3.14), string_val("Server") {
    vector_val.push_back(-1);
    map_val.insert(std::make_pair("key", -1));
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
  Derived() : Base(), derived_val(1) {}
  ~Derived() {}

  int derived_val;

  // magic(same as MSGPACK_DEFINE)
  LINEAR_PACK(int_val, double_val, string_val, vector_val, map_val, // Base::
              derived_val);
};

// You need to implement concrete handler class like as follows.
class ApplicationHandler : public linear::Handler {
 public:
  ApplicationHandler() {}
  ~ApplicationHandler() {}
  void OnConnect(const linear::Socket& socket) {
    const linear::Addrinfo& info = socket.GetPeerInfo();
    std::cout << "OnConnect: " << info.addr << ":" << info.port << std::endl;

    // SSLSocket specific
    linear::SSLSocket ssl_socket = socket.as<linear::SSLSocket>();
    std::vector<linear::X509Certificate> certs = ssl_socket.GetPeerCertificateChain();
    for (std::vector<linear::X509Certificate>::iterator it = certs.begin();
         it != certs.end(); it++) {
      std::cout << ( (it->IsCA()) ? "Intermediate CA" : "Client" )
                << ", Subject DN: " << it->GetSubject().DN
                << ", Issuer DN: " << it->GetIssuer().DN
                << std::endl;
    }
    linear::X509Certificate cert = ssl_socket.GetPeerCertificate();
    std::cout << ( (cert.IsCA()) ? "Intermediate CA" : "Client" )
              << ", Subject DN: " << cert.GetSubject().DN
              << ", Issuer DN: " << cert.GetIssuer().DN
              << std::endl;
    linear::Error err = ssl_socket.GetVerifyResult();
    if (err.Code() != linear::LNR_OK) {
      std::cerr << "invalid ClientCertificate" << std::endl;
      socket.Disconnect();
    }
    // SSLSocket specific end
  }
  void OnDisconnect(const linear::Socket& socket, const linear::Error&) {
    const linear::Addrinfo& info = socket.GetPeerInfo();
    std::cout << "OnDisconnect: " << info.addr << ":" << info.port << std::endl;
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
        if (request.method == "echo") {
          std::cout << "do echo back: " << request.params.stringify() << std::endl;
          linear::Response response(request.msgid, request.params);
          response.Send(socket);
        } else {
          linear::Response response(request.msgid, linear::type::nil(), std::string("method not found"));
          response.Send(socket);
        }
      }
      break;
    case linear::RESPONSE:
      {
        linear::Response response = msg.as<linear::Response>();
        std::cout << "recv Response: msgid = " << response.msgid
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
        linear::Notify notify_from_server("from server", Derived());
        notify_from_server.Send(LINEAR_BROADCAST_GROUP); // send notify to all connected clients
      }
      break;
    default:
      {
        std::cout << "BUG: plz inform to linear-developpers" << std::endl;
      }
      break;
    }
  }
  void OnError(const linear::Socket&, const linear::Message& msg, const linear::Error& err) {
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
};

void usage(char* name) {
  std::cout << "Usage: " << std::string(name) << " [Host := 0.0.0.0] [Port := 37800]" << std::endl;
}

int main(int argc, char* argv[]) {

#if _WIN32
  linear::log::SetLevel(linear::log::LOG_DEBUG);
  linear::log::EnableStderr();
  std::string host = (argc > 1) ? std::string(argv[1]) : "0.0.0.0";
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
  std::string host = (argc >= 1) ? std::string(argv[0]) : "0.0.0.0";
  int port = (argc >= 2) ? atoi(argv[1]) : 37800;
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  linear::SSLContext ssl_context(linear::SSLContext::TLSv1_1);
#else
  linear::SSLContext ssl_context(linear::SSLContext::TLS);
#endif
  bool ret = ssl_context.SetCertificate(std::string(SERVER_CERT));
  if (!ret) {
    std::cerr << "SetCertificate error" << std::endl;
    return -1;
  }
  ret = ssl_context.SetPrivateKey(std::string(SERVER_PRIVATE_KEY));
  if (!ret) {
    std::cerr << "SetPrivateKey error" << std::endl;
    return -1;
  }
  ret = ssl_context.SetCAFile(std::string(CA_CERT));
  if (!ret) {
    std::cerr << "SetCAFile error" << std::endl;
    return -1;
  }
  ret = ssl_context.SetCiphers(std::string("ALL:EECDH+HIGH:EDH+HIGH:+MEDIUM+HIGH:!EXP:!LOW:!eNULL:!aNULL:!MD5:!RC4:!ADH:!KRB5:!PSK:!SRP"));
  if (!ret) {
    std::cerr << "SetCiphers error" << std::endl;
    return -1;
  }
  ssl_context.SetVerifyMode(linear::SSLContext::VERIFY_PEER);

  linear::shared_ptr<ApplicationHandler> handler = linear::shared_ptr<ApplicationHandler>(new ApplicationHandler());
  linear::SSLServer server(handler, ssl_context);
  server.SetMaxClients(5); // limit 5 clients
  server.Start(host, port);

  std::cout << "press enter to exit" << std::endl;
  std::string parameter;
  std::getline(std::cin, parameter);

  server.Stop();

  return 0;
}
