#pragma once

#include "l3/l3.h"

namespace pol4b {

  class RouteInfo {
  public:
    RouteInfo() = default;
    IPv4Addr gateway;
    IPv4Addr destination;
    IPv4Addr prefsrc;
    SubnetMask mask;
    uint32_t metric = 0;
  };

};
