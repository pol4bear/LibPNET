#include "l3/ipv4.h"
#include <gtest/gtest.h>
#include <arpa/inet.h>

using namespace std;
using namespace pol4b;

TEST(IPv4Test, BasicAssertions) {
  ASSERT_THROW(IPv4Addr("1"), invalid_argument);
  ASSERT_THROW(IPv4Addr("1111111111111111"), invalid_argument);
  ASSERT_THROW(IPv4Addr("19216801"), invalid_argument);
  ASSERT_THROW(IPv4Addr("192.168.0"), invalid_argument);
  ASSERT_EQ(sizeof(IPv4Addr), 4);
  ASSERT_EQ((uint32_t)IPv4Addr("192.168.0.1"), 0xC0A80001);
  ASSERT_EQ((string)IPv4Addr(0xC0A80001), "192.168.0.1");
  IPv4Addr a("192.168.0.1");
  IPv4Addr b;
  a.copy((uint8_t*)&b, true);
  if (htonl(1) == 1)
    ASSERT_EQ((string)b, "192.168.0.1");
  else
    ASSERT_EQ((string)b, "1.0.168.192");
  a += 10;
  ASSERT_EQ((string)a, "192.168.0.11");
  a++;
  ASSERT_EQ((string)a, "192.168.0.12");
  ASSERT_EQ((string)++a, "192.168.0.13");
}
