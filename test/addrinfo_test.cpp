#include "gtest/gtest.h"

#include "test_common.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string>
#include <iostream>

#include "linear/addrinfo.h"

typedef LinearTest AddrinfoTest;

TEST_F(AddrinfoTest, all) {
  std::string test_addr("192.168.0.1");
  int test_port = 10000;

  linear::Addrinfo ai = linear::Addrinfo(test_addr, test_port);
  ASSERT_EQ(ai.addr, test_addr);
  ASSERT_EQ(ai.port, test_port);
  ASSERT_EQ(ai.proto, linear::Addrinfo::IPv4);

  std::ostringstream os;
  struct addrinfo hints;
  struct addrinfo *res;

  test_addr = "192.168.0.2";
  test_port = 10001;
  os << test_port;

  memset(&hints, 0, sizeof(hints));
  getaddrinfo(test_addr.c_str(), os.str().c_str(), &hints, &res);
  ai = linear::Addrinfo(res->ai_addr);
  ASSERT_EQ(ai.addr, test_addr);
  ASSERT_EQ(ai.port, test_port);
  ASSERT_EQ(ai.proto, linear::Addrinfo::IPv4);

  test_addr = "::1";
  test_port = 10002;
  
  ai = linear::Addrinfo(test_addr, test_port);
  ASSERT_EQ(ai.addr, test_addr);
  ASSERT_EQ(ai.port, test_port);
  ASSERT_EQ(ai.proto, linear::Addrinfo::IPv6);

  os.str("");
  os.clear(std::stringstream::goodbit);
  os << test_port;

  memset(&hints, 0, sizeof(hints));
  getaddrinfo(test_addr.c_str(), os.str().c_str(), &hints, &res);
  ai = linear::Addrinfo(res->ai_addr);
  ASSERT_EQ(ai.addr, test_addr);
  ASSERT_EQ(ai.port, test_port);
  ASSERT_EQ(ai.proto, linear::Addrinfo::IPv6);
}
