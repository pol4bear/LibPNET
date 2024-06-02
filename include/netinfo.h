#pragma once

#include "l2/mac.h"
#include "l3/ipv4.h"
#include "l3/subnetmask.h"
#include <vector>

namespace pol4b {

class NetInfo {
public:
  NetInfo() = default;
  MACAddr mac;
  IPv4Addr ip;
  SubnetMask mask;
};

};
