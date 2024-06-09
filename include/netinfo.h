#pragma once

#include "l2/mac.h"
#include "l3/ipv4.h"
#include "l3/subnetmask.h"
#include <vector>

namespace pol4b {

/**
 * @brief The NetInfo class.
 *
 * This class represents network information, including the MAC address,
 * IPv4 address, and subnet mask.
 */
class NetInfo {
public:
  /**
   * @brief Construct a new NetInfo object with default values.
   */
  NetInfo() = default;

  /**
   * @brief The MAC address of the network interface.
   */
  MACAddr mac;

  /**
   * @brief The IPv4 address of the network interface.
   */
  IPv4Addr ip;

  /**
   * @brief The subnet mask of the network interface.
   */
  SubnetMask mask;
};


};
