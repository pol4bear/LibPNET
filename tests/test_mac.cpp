#include "l2/mac.h"
#include <gtest/gtest.h>
#include <arpa/inet.h>

using namespace std;
using namespace pol4b;

TEST(MACTest, BasicAssertions) {
  ASSERT_THROW(MACAddr("1"), invalid_argument);
  ASSERT_THROW(MACAddr("1111111111111111"), invalid_argument);
  ASSERT_THROW(MACAddr("ABCDEFGHIJKL"), invalid_argument);
  ASSERT_THROW(MACAddr("AB:CD:EF:GH:IJ:KL"), invalid_argument);
  ASSERT_THROW(MACAddr("AB-CD-EF-GH-IJ-KL"), invalid_argument);
  ASSERT_EQ(sizeof(MACAddr), 6);
  ASSERT_EQ((uint64_t)MACAddr("AA:BB:CC:DD:EE:FF"), 0xAABBCCDDEEFF);
  ASSERT_EQ((uint64_t)MACAddr("AA-BB-CC-DD-EE-FF"), 0xAABBCCDDEEFF);
  ASSERT_EQ((string)MACAddr(0xAABBCCDDEEFF), "AA:BB:CC:DD:EE:FF");
  MACAddr a("AA:BB:CC:DD:EE:FF");
  MACAddr b;
  a.copy((uint8_t*)&b, true);
  if (htonl(1) == 1)
    ASSERT_EQ((string)b, "AA:BB:CC:DD:EE:FF");
  else
    ASSERT_EQ((string)b, "FF:EE:DD:CC:BB:AA");
  a += 10;
  ASSERT_EQ((string)a, "AA:BB:CC:DD:EF:09");
  a++;
  ASSERT_EQ((string)a, "AA:BB:CC:DD:EF:0A");
  ASSERT_EQ((string)++a, "AA:BB:CC:DD:EF:0B");
}
