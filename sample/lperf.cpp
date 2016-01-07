// linear performance checker

#include <unistd.h>
#include <sys/time.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>

#include "linear/condition_variable.h"
#include "linear/tcp_server.h"
#include "linear/tcp_client.h"
#include "linear/log.h"

#define DEFAULT_TRY_NUM (1000)
#define DEFAULT_MSIZ (128)

using namespace linear::log;

namespace receiver {

class Handler : public linear::Handler {
 public:
  Handler() {}
  ~Handler() {}

  void OnConnect(const linear::Socket&) {
    linear::unique_lock<linear::mutex> lock(mutex_);
    cv_.notify_one();
  }
  void OnDisconnect(const linear::Socket&, const linear::Error&) {
    linear::unique_lock<linear::mutex> lock(mutex_);
    cv_.notify_one();
  }
  void OnMessage(const linear::Socket& socket, const linear::Message& msg) {
    switch(msg.type) {
    case linear::REQUEST:
      {
        const linear::Request& request = msg.as<linear::Request>();
        if (request.method == "echo") {
          linear::Response response(request.msgid, request.params);
          response.Send(socket);
        } else {
          linear::Response response(request.msgid, linear::type::nil(), std::string("invalid method"));
          response.Send(socket);
        }
        break;
      }
    case linear::NOTIFY:
    case linear::RESPONSE:
    default:
      {
        break;
      }
    }
  }
  void WaitToFinish() {
    linear::unique_lock<linear::mutex> lock(mutex_);
    cv_.wait(lock);
  }

 private:
  linear::mutex mutex_;
  linear::condition_variable cv_;
};

} // namespace receiver

namespace sender {

class Handler : public linear::Handler {
 public:
  Handler(size_t num, size_t msiz) : num_(num), msiz_(msiz) {}
  ~Handler() {}

  void OnConnect(const linear::Socket& socket) {
    gettimeofday(&prev_, NULL);
    linear::Request request("echo", std::string(msiz_, 'a'));
    linear::Error e = request.Send(socket);
    if (e.Code() != linear::LNR_OK) {
      socket.Disconnect();
    }
  }
  void OnDisconnect(const linear::Socket&, const linear::Error&) {
    linear::unique_lock<linear::mutex> lock(mutex_);
    cv_.notify_one();
  }
  void OnMessage(const linear::Socket& socket, const linear::Message& msg) {
    switch(msg.type) {
    case linear::RESPONSE:
      {
        linear::Response response = msg.as<linear::Response>();
        struct timeval t;
        gettimeofday(&t, NULL);
        uint64_t d = (t.tv_sec - prev_.tv_sec) * 1000 * 1000 + (t.tv_usec - prev_.tv_usec);
        duration_.push_back(d);
        prev_ = t;
        num_--;
        if (num_ == 0) {
          socket.Disconnect();
          return;
        }
        linear::Request request("echo", std::string(msiz_, 'a'));
        linear::Error e = request.Send(socket);
        if (e.Code() != linear::LNR_OK) {
          linear::unique_lock<linear::mutex> lock(mutex_);
          cv_.notify_one();
          return;
        }
        break;
      }
    case linear::NOTIFY:
    case linear::REQUEST:
    default:
      {
        break;
      }
    }
  }
  void OnError(const linear::Socket& socket, const linear::Message&, const linear::Error&) {
    socket.Disconnect();
  }

  bool WaitToFinish() {
    linear::unique_lock<linear::mutex> lock(mutex_);
    cv_.wait(lock);
    return (num_ == 0);
  }
  void ShowResult() {
    uint64_t max = *(duration_.begin()), min = *(duration_.begin()), ave;
    for (std::vector<uint64_t>::iterator it = duration_.begin(); it != duration_.end(); it++) {
      min = (*it < min) ? *it : min;
      max = (max > *it) ? max : *it;
    }
    ave = std::accumulate(duration_.begin(), duration_.end(), 0) / duration_.size();
    size_t midIndex = duration_.size() / 2;
    std::nth_element(duration_.begin(), duration_.end() + midIndex, duration_.end());
    std::cout << "--- Result ---" << std::endl;
    std::cout << "success: " << duration_.size() << ", RTT => "
              << "min: " << min / 1000.0 << "ms, "
              << "max: " << max / 1000.0 << "ms, "
              << "ave: " << ave / 1000.0 << "ms, "
              << "med: " << duration_[midIndex] / 1000.0 << "ms" << std::endl;
  }

 private:
  size_t num_;
  size_t msiz_;
  struct timeval prev_;
  std::vector<uint64_t> duration_;
  linear::mutex mutex_;
  linear::condition_variable cv_;
};

} // namespace sender

void usage(char* name) {
  std::cout << "linear performance checker by using simple echo." << std::endl;
  std::cout << "run 'as a server' at first, and next run 'as a client'." << std::endl << std::endl;
  std::cout << "Usage: " << std::string(name) << " [options] [Host := 0.0.0.0] [Port := 10000]" << std::endl;
  std::cout << "[Mandatory option]" << std::endl;
  std::cout << "Please choose one from the following:" << std::endl;
  std::cout << "  -cs     : Run as a client to send a request       [  Sender  ]" << std::endl;
  std::cout << "  -cr     : Run as a client to receive a request    [ Receiver ]" << std::endl;
  std::cout << "  -ss     : Run as a server to send a request       [  Sender  ]" << std::endl;
  std::cout << "  -sr     : Run as a server to receive a request    [ Receiver ]" << std::endl;
  std::cout << "[Sender option]" << std::endl;
  std::cout << "  -m Size : Set message size.                       default := 128bytes" << std::endl;
  std::cout << "  -n Num  : Set num of try.                         default := 1000times" << std::endl;
  std::cout << "[Debug option]" << std::endl;
  std::cout << "  -l Level: Show log.                               default := off" << std::endl;
  std::cout << "            ERR = 0, WARN = 1, INFO = 2, DEBUG = 3, FULL = 4" << std::endl;
}

int main(int argc, char* argv[]) {
  typedef enum {
    UNDEFINED,
    CLIENT,
    SERVER,
  } NodeType;

  typedef enum {
    SENDER,
    RECEIVER,
  } NodeMode;

  int ch, l;
  extern char* optarg;
  extern int optind;

  char t = '\0';
  NodeType type = UNDEFINED;
  NodeMode mode = SENDER;
  size_t num = DEFAULT_TRY_NUM, msiz = DEFAULT_MSIZ;
  linear::log::Level level = linear::log::LOG_OFF;

  while ((ch = getopt(argc, argv, "c:l:m:n:s:")) != -1) {
    switch(ch) {
    case 'c':
      type = CLIENT;
      t = *optarg;
      break;
    case 'l':
      l = atoi(optarg);
      if (l >= 0) {
        level = (l < 4) ? static_cast<linear::log::Level>(l) : LOG_FULL;
      }
      break;
    case 'm':
      msiz = atoi(optarg);
      msiz = (msiz <= 0) ? DEFAULT_MSIZ : msiz;
      break;
    case 'n':
      num = atoi(optarg);
      num = (num <= 0) ? DEFAULT_TRY_NUM : num;
      break;
    case 's':
      type = SERVER;
      t = *optarg;
      break;
    default:
      usage(argv[0]);
      return -1;
    }
  }
  if (type == UNDEFINED) {
    usage(argv[0]);
    return -1;
  }
  switch(t) {
  case 's':
    mode = SENDER;
    break;
  case 'r':
    mode = RECEIVER;
    break;
  default:
    usage(argv[0]);
    return -1;
  }

  argc -= optind;
  argv += optind;

  std::string host = (argc >= 1) ? std::string(argv[0]) : "127.0.0.1";
  int port = (argc >= 2) ? atoi(argv[1]) : 10000;
  std::string tmp;

  if (level != LOG_OFF) {
    linear::log::SetLevel(level);
    linear::log::EnableStderr();
  }

  if (type == CLIENT) {
    switch(mode) {
    case RECEIVER:
      {
        linear::shared_ptr<receiver::Handler> h = linear::shared_ptr<receiver::Handler>(new receiver::Handler());
        linear::TCPClient c(h);
        linear::TCPSocket s = c.CreateSocket(host, port);
        s.Connect();
        h->WaitToFinish();
        if (s.GetState() == linear::Socket::CONNECTED) {
          std::cout << "Run as a client to receive a request" << std::endl;
          std::cout << "--- Conditions ---" << std::endl;
          std::cout << "Target server: " << host << ":" << port << std::endl;
          h->WaitToFinish();
        } else {
          std::cerr << "perf fail" << std::endl;
        }
        break;
      }
    case SENDER:
      {
        std::cout << "Run as a client to send a request" << std::endl;
        std::cout << "--- Conditions ---" << std::endl;
        std::cout << "Target: " << host << ":" << port
                  << ", Num of try: " << num << ", Message size: " << msiz << "bytes" << std::endl;
        linear::shared_ptr<sender::Handler> h = linear::shared_ptr<sender::Handler>(new sender::Handler(num, msiz));
        linear::TCPClient c(h);
        linear::TCPSocket s = c.CreateSocket(host, port);
        s.Connect();
        if (h->WaitToFinish()) {
          h->ShowResult();
        } else {
          std::cerr << "perf fail" << std::endl;
        }
        break;
      }
    default:
      assert(false);
    }
  } else if (type == SERVER) {
    switch (mode) {
    case RECEIVER:
      {
        linear::shared_ptr<receiver::Handler> h = linear::shared_ptr<receiver::Handler>(new receiver::Handler());
        linear::TCPServer s(h);
        s.Start(host, port);
        std::cout << "Run as a server to receive a request" << std::endl;
        std::cout << "--- Conditions ---" << std::endl;
        std::cout << "Server started: " << host << ":" << port << std::endl;
        h->WaitToFinish();
        h->WaitToFinish();
        break;
      }
    case SENDER:
      {
        std::cout << "Run as a server to send a request" << std::endl;
        std::cout << "--- Conditions ---" << std::endl;
        std::cout << "Server started: " << host << ":" << port
                  << ", Num of try: " << num << ", Message size: " << msiz << "bytes" << std::endl;
        linear::shared_ptr<sender::Handler> h = linear::shared_ptr<sender::Handler>(new sender::Handler(num, msiz));
        linear::TCPServer s(h);
        s.Start(host, port);
        if (h->WaitToFinish()) {
          h->ShowResult();
        } else {
          std::cerr << "perf fail" << std::endl;
        }
        break;
      }
    default:
      assert(false);
    }
  }
  return 0;
}
