#include "l3/subnetmask.h"
#include <gtest/gtest.h>

using namespace std;
using namespace pol4b;

TEST(CIDRTest, BasicAssertions) {
  ASSERT_EQ((string)SubnetMask::from_cidr(8), "255.0.0.0");
  ASSERT_EQ((string)SubnetMask::from_cidr(16), "255.255.0.0");
  ASSERT_EQ((string)SubnetMask::from_cidr(20), "255.255.240.0");
  ASSERT_EQ((string)SubnetMask::from_cidr(24), "255.255.255.0");
  ASSERT_EQ((string)SubnetMask::from_cidr(32), "255.255.255.255");
  ASSERT_EQ(SubnetMask("255.0.0.0").to_cidr(), 8);
  ASSERT_EQ(SubnetMask("255.255.0.0").to_cidr(), 16);
  ASSERT_EQ(SubnetMask("255.255.240.0").to_cidr(), 20);
  ASSERT_EQ(SubnetMask("255.255.255.0").to_cidr(), 24);
  ASSERT_EQ(SubnetMask("255.255.255.255").to_cidr(), 32);
}
